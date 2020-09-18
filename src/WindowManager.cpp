#include "WindowManager.h"

#include <stdio.h>
#include <X11/cursorfont.h>

namespace ZWM
{
    WindowManager *WindowManager::m_instance = nullptr;

    WindowManager *WindowManager::the()
    {
        if (!m_instance)
            m_instance = new WindowManager();
        return m_instance;
    }

    Window WindowManager::find_frame_for_xwindow(Window w)
    {
        for (auto value : m_frames_to_framedwindows)
        {
            if (value.second->framed_window() == w)
                return value.first;
        }
        return 0;
    }

    int WindowManager::init()
    {
        if (!(m_display = XOpenDisplay(0)))
        {
            fprintf(stderr, "Failed to open display!\n");
            return 1;
        }

        XGrabButton(m_display, Button1Mask, Mod1Mask | Mod2Mask, DefaultRootWindow(m_display), True,
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
        XGrabButton(m_display, Button3Mask, Mod1Mask | Mod2Mask, DefaultRootWindow(m_display), True,
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

        // To get maprequest events
        XSelectInput(
            m_display,
            DefaultRootWindow(m_display),
            SubstructureRedirectMask | SubstructureNotifyMask);

        XSync(m_display, false);

        // Get existing windows to frame them

        XGrabServer(m_display);
        Window tree_root, tree_parent;
        Window *existing_windows;
        unsigned int existing_windows_size;

        XQueryTree(m_display, DefaultRootWindow(m_display),
                   &tree_root, &tree_parent,
                   &existing_windows, &existing_windows_size);

        for (size_t i = 0; i < existing_windows_size; i++)
        {
            auto *framed_window = new ZWM::FramedWindow(m_display, existing_windows[i]);
            m_frames_to_framedwindows[framed_window->frame()] = framed_window;
        }

        XFree(existing_windows);
        XUngrabServer(m_display);
        auto default_cursor = XCreateFontCursor(m_display, XC_arrow);
        XDefineCursor(m_display, DefaultRootWindow(m_display), default_cursor);
        return 0;
    }

    void WindowManager::run_loop()
    {
        // Loop initialization

        ZWM::Position last_cursor_position{};
        XEvent event;
        XWindowAttributes attr{};

        fprintf(stdout, "ZWM initialized! Starting event loop\n");

        // Loop

        while (true)
        {
            XNextEvent(m_display, &event);

            switch (event.type)
            {
            case ButtonPress:
            {
                // TODO: Cleanup, this could be made waaaay nicer…
                if (event.xbutton.subwindow) // clicked on a window
                {
                    auto window = event.xbutton.subwindow;
                    XGetWindowAttributes(m_display, window, &attr);
                    last_cursor_position = {event.xbutton.x_root, event.xbutton.y_root};

                    XRaiseWindow(m_display, window);
                    XSetInputFocus(m_display, window, RevertToParent, CurrentTime);
                }
                if (event.xbutton.window != event.xbutton.root) // Clicked on a window's frame
                {
                    auto window = event.xbutton.window;
                    XGetWindowAttributes(m_display, window, &attr);
                    last_cursor_position = {event.xbutton.x_root, event.xbutton.y_root};

                    XRaiseWindow(m_display, window);
                    XSetInputFocus(m_display, window, RevertToParent, CurrentTime);
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
                    XGetWindowAttributes(m_display, window, &win_attrs);

                    ZWM::Position window_position{win_attrs.x, win_attrs.y};

                    if (!m_frames_to_framedwindows.count(window))
                    {
                        fprintf(stderr, "ERR: Motion on invalid window!\n");
                        break;
                    }

                    int xdiff = current_cursor_position.x - last_cursor_position.x;
                    int ydiff = current_cursor_position.y - last_cursor_position.y;

                    auto framed_window = m_frames_to_framedwindows[window];
                    if (event.xbutton.state & Mod1Mask) // If ALT is pressed
                    {                                   // We handle keybinding for position-agnostic actions

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
                if (event.xbutton.window != event.xbutton.root) // Event on a non-root window (a frame!)
                {
                    auto window = event.xbutton.window;
                    if (event.xbutton.state & Button1Mask)
                    {
                        int xdiff = current_cursor_position.x - last_cursor_position.x;
                        int ydiff = current_cursor_position.y - last_cursor_position.y;

                        auto framed_window = m_frames_to_framedwindows[window];

                        ZWM::Position new_pos = framed_window->pos();
                        new_pos.x += xdiff;
                        new_pos.y += ydiff;
                        framed_window->move(new_pos);
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
                XMapWindow(m_display, event_window);                                  // Actually map the window
                auto *framed_window = new ZWM::FramedWindow(m_display, event_window); // Create a frame for the window
                m_frames_to_framedwindows[framed_window->frame()] = framed_window;    // save it
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

                if (event.xunmap.event == DefaultRootWindow(m_display))
                {
                    // I have no idea why or how this happens, just copy-pasted this from
                    // https://github.com/jichu4n/basic_wm/blob/75483547ae0ddb7585c28af86b9c957ba6c3302b/window_manager.cpp#L310
                    // And it works…
                    // The `event` property isn't even documented in the man page, I have no idea what this does, but it prevents
                    // windows that shouldn't be unmapped from being unmapped, so it works I guess
                    fprintf(stderr, "Ignoring unmap notification for window %u\n", event_window);
                    break;
                }
                delete m_frames_to_framedwindows[frame];
                break;
            }

            case ConfigureRequest:
            {
                auto config_request = event.xconfigurerequest;
                auto window = config_request.window;
                XWindowChanges changes{};
                changes.x = config_request.x;
                changes.y = config_request.y;
                changes.width = config_request.width;
                changes.height = config_request.height;
                changes.border_width = config_request.border_width;

                Window windows_frame = find_frame_for_xwindow(window);
                if (windows_frame)
                {
                    auto framed_window = m_frames_to_framedwindows[windows_frame];
                    ZWM::Position new_pos{changes.x, changes.y};
                    ZWM::Size new_size{changes.width, changes.height};

                    if (new_pos != framed_window->pos())
                        framed_window->move(new_pos);
                    if (new_size != framed_window->size())
                        framed_window->resize(new_size);
                }

                XConfigureWindow(m_display, window, config_request.value_mask, &changes);
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
} // namespace ZWM
