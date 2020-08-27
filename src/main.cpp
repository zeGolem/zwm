#include <X11/Xlib.h>
#include <unordered_map>
#include <stdio.h>

#include "FramedWindow.h"

Display *display;

std::unordered_map<Frame, ZWM::FramedWindow *> frames_to_framedwindows;

Frame find_frame_for_xwindow(Window w)
{
	for (auto value : frames_to_framedwindows)
	{
		fprintf(stdout, "checking window %u against %u\n", w, value.second->framed_window());
		if (value.second->framed_window() == w)
			return value.first;
	}
	fprintf(stderr, "Didn't find a frame for window %u\n", w);
	return 0;
}

int main(void)
{
	fprintf(stdout, "Hello from zwm!\n");

	if (!(display = XOpenDisplay(0x0)))
		return 1;

	XGrabKey(display, XKeysymToKeycode(display, XStringToKeysym("F1")), Mod1Mask | Mod2Mask,
			 DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
	XGrabButton(display, Button1Mask | Button3Mask, AnyModifier, DefaultRootWindow(display), True,
				ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, 0, 0);

	// To get maprequest events
	XSelectInput(
		display,
		DefaultRootWindow(display),
		SubstructureRedirectMask | SubstructureNotifyMask);

	XSync(display, false);

	// Get existing windows to frame them

	XGrabServer(display);
	Window tree_root, tree_parent;
	Window *existing_windows;
	unsigned int existing_windows_size;

	XQueryTree(display, DefaultRootWindow(display),
			   &tree_root, &tree_parent,
			   &existing_windows, &existing_windows_size);

	for (size_t i = 0; i < existing_windows_size; i++)
	{
		auto *framed_window = new ZWM::FramedWindow(display, existing_windows[i]);
		frames_to_framedwindows[framed_window->frame()] = framed_window;
	}

	XFree(existing_windows);
	XUngrabServer(display);

	// Loop initialization

	ZWM::Position last_cursor_position{};
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
				auto window = event.xbutton.subwindow;
				XGetWindowAttributes(display, window, &attr);
				last_cursor_position = {event.xbutton.x_root, event.xbutton.y_root};
				XRaiseWindow(display, window);
				XSetInputFocus(display, window, RevertToParent, CurrentTime);
			}
			break;
		}

		case MotionNotify:
		{
			ZWM::Position current_cursor_position{event.xbutton.x_root, event.xbutton.y_root};
			if (event.xbutton.subwindow)
			{
				// The cursor is on this window. This should actually be the window's frame.
				auto window = event.xbutton.subwindow;

				XWindowAttributes win_attrs{};
				XGetWindowAttributes(display, window, &win_attrs);

				ZWM::Position window_position{win_attrs.x, win_attrs.y};

				if (!frames_to_framedwindows.count(window))
				{
					fprintf(stderr, "ERR: Motion on invalid window!\n");
					break;
				}

				int xdiff = current_cursor_position.x - last_cursor_position.x;
				int ydiff = current_cursor_position.y - last_cursor_position.y;

				auto framed_window = frames_to_framedwindows[window];
				if (event.xbutton.state & Mod1Mask) // If ALT is pressed
				{									// We handle keybinding for position-agnostic actions

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
				else
				{
					if (current_cursor_position.y - window_position.y < framed_window->top_bar_size()) // if cursor is in the top bar
					{
						if (event.xbutton.state & Button1Mask) // if is pressing left click
						{									   // move the window
							ZWM::Position new_pos = framed_window->pos();
							new_pos.x += xdiff;
							new_pos.y += ydiff;
							framed_window->move(new_pos);
						}
					}
				}
			}

			last_cursor_position = current_cursor_position;
			break;
		}

		case ButtonRelease:
		{
			last_cursor_position = ZWM::Position{};
			break;
		}

		case MapRequest:
		{
			Window event_window = event.xmaprequest.window;
			auto *framed_window = new ZWM::FramedWindow(display, event_window); // Create a frame for the window
			frames_to_framedwindows[framed_window->frame()] = framed_window;	// save it
			XMapWindow(display, event_window);									// Actually map the window
			break;
		}

		case UnmapNotify:
		{
			auto event_window = event.xunmap.window;

			auto frame = find_frame_for_xwindow(event_window);
			if (frame == 0)
			{
				fprintf(stderr, "Unmapping a window that was not framed! Ignoring…\n");
				break;
			}

			if (event.xunmap.event == DefaultRootWindow(display))
			{
				// I have no idea why or how this happens, just copy-pasted this from
				// https://github.com/jichu4n/basic_wm/blob/75483547ae0ddb7585c28af86b9c957ba6c3302b/window_manager.cpp#L310
				// And it works…
				// The `event` property isn't even documented in the man page, I have no idea what this does, but it prevents
				// windows that shouldn't be unmapped from being unmapped, so it works I guess
				fprintf(stderr, "Ignoring unmap notification for window %u\n", event_window);
				break;
			}
			delete frames_to_framedwindows[frame];
			break;
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

		// Events to ignore.
		case CreateNotify:
		case DestroyNotify:
		case ConfigureNotify:
		case MapNotify:
		case ReparentNotify:
			break;

		default:
			fprintf(stdout, "Unhandled event %d\n", event.type);
			break;
		}
	}
}
