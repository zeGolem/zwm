#include <X11/Xlib.h>
#include <algorithm>
#include <unordered_map>
#include <vector>
#include <stdio.h>

#include "FramedWindow.h"

Display *display;

// std::unordered_map<Window, Frame> windows_to_frames;
std::unordered_map<Frame, ZWM::FramedWindow *> frames_to_framedwindows;
// std::vector<ZWM::FramedWindow> windows;

int main(void)
{
	fprintf(stdout, "Hello from zwm!\n");

	if (!(display = XOpenDisplay(0x0)))
		return 1;

	XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F1")), Mod1Mask | Mod2Mask,
			 DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
	XGrabButton(display, 1, Mod1Mask | Mod2Mask, DefaultRootWindow(display), True,
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, 0, 0);
	XGrabButton(display, 3, Mod1Mask | Mod2Mask, DefaultRootWindow(display), True,
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, 0, 0);

	// To get maprequest events
	XSelectInput(
		display,
		DefaultRootWindow(display),
		SubstructureRedirectMask | SubstructureNotifyMask);

	XSync(display, false);

	ZWM::Position last_position{};
	XEvent event;
	XWindowAttributes attr;

	fprintf(stdout, "ZWM initialized! Starting event loop\n");

	for (;;)
	{
		XNextEvent(display, &event);

		switch (event.type)
		{
		case KeyPress:
		{
			if (event.xkey.subwindow)
				XRaiseWindow(display, event.xkey.subwindow);
			break;
		}

		case ButtonPress:
		{
			if (event.xbutton.subwindow)
			{
				XGetWindowAttributes(display, event.xbutton.subwindow, &attr);
				last_position = {event.xbutton.x_root, event.xbutton.y_root};
			}
			break;
		}

		case MotionNotify:
		{
			ZWM::Position current_position{event.xbutton.x_root, event.xbutton.y_root};
			if (event.xbutton.subwindow)
			{
				auto window = event.xbutton.subwindow;
				int xdiff = current_position.x - last_position.x;
				int ydiff = current_position.y - last_position.y;

				if (!frames_to_framedwindows.count(window)) // Deprecated. Will be removed once all window are FramedWIndows
				{
					fprintf(stderr, "WARN/DEPRECATED: Moving an unframed window!\n");
					XMoveResizeWindow(display, window,
									  attr.x + (event.xbutton.button == 1 ? xdiff : 0),
									  attr.y + (event.xbutton.button == 1 ? ydiff : 0),
									  std::max(1, attr.width + (event.xbutton.button == 3 ? xdiff : 0)),
									  std::max(1, attr.height + (event.xbutton.button == 3 ? ydiff : 0)));
				}
				else
				{
					auto framed_window = frames_to_framedwindows[window];

					if (event.xbutton.state & Button1Mask) // left click; move
					{
						ZWM::Position new_pos = framed_window->pos();
						new_pos.x += xdiff;
						new_pos.y += ydiff;
						framed_window->move(new_pos);
					}
					else if (event.xbutton.state & Button3Mask) // right click; resize
					{
						ZWM::Size new_size = framed_window->size();
						new_size.width += xdiff;
						new_size.height += ydiff;
						framed_window->resize(new_size);
					}
				}
			}

			last_position = current_position;
			break;
		}

		case ButtonRelease:
		{
			last_position = ZWM::Position{};
			break;
		}

		case MapRequest:
		{
			Window event_window = event.xmaprequest.window;
			ZWM::FramedWindow *framed_window = new ZWM::FramedWindow(display, event_window);
			frames_to_framedwindows[framed_window->frame()] = framed_window;
			XMapWindow(display, event_window);
		}

		case ConfigureRequest:
		{
			auto config_request = event.xconfigurerequest;
			XWindowChanges changes{};
			changes.x = config_request.x;
			changes.y = config_request.y;
			changes.width = config_request.width;
			changes.height = config_request.height;
			changes.border_width = config_request.border_width;
			XConfigureWindow(display, config_request.window, config_request.value_mask, &changes);
			break;
		}

		default:
			fprintf(stdout, "Unhandled event %d\n", event.type);
			break;
		}
	}
}
