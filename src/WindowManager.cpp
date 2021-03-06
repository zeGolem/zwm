#include "WindowManager.h"
#include "FramedWindow.h"

#include <bits/stdint-uintn.h>
#include <cstdlib>
#include <stdio.h>
#include <xcb/xcb.h>
#include <xcb/xcb_cursor.h>
#include <xcb/xproto.h>

namespace ZWM
{
WindowManager *WindowManager::m_instance = nullptr;

WindowManager *WindowManager::the()
{
	if (!m_instance) m_instance = new WindowManager();
	return m_instance;
}

xcb_window_t WindowManager::find_frame_for_xwindow(xcb_window_t w)
{
	for (auto value : m_frames_to_framedwindows) {
		if (value.second->framed_window() == w) return value.first;
	}
	return 0;
}

/*
ulong WindowManager::black_pixel()
{
    if (!m_black_pixel)
        m_black_pixel = BlackPixel(m_display, DefaultScreen(m_display));
    return m_black_pixel;
}

ulong WindowManager::white_pixel()
{
    if (!m_white_pixel)
        m_white_pixel = WhitePixel(m_display, DefaultScreen(m_display));
    return m_white_pixel;
}

XFontStruct *WindowManager::default_font()
{
    if (!m_default_font)
        m_default_font = XLoadQueryFont(m_display, "fixed");
    return m_default_font;
}

void WindowManager::init_atoms()
{
#define REGISTER_ATOM(atom) m_atoms[atom] = XInternAtom(m_display, atom, false);
    REGISTER_ATOM("UTF8_STRING");
    REGISTER_ATOM("WM_PROTOCOLS");
    REGISTER_ATOM("WM_DELETE_WINDOW");
    REGISTER_ATOM("WM_STATE");
    REGISTER_ATOM("WM_TAKE_FOCUS");
    REGISTER_ATOM("_NET_ACTIVE_WINDOW");
    REGISTER_ATOM("_NET_SUPPORTED");
    REGISTER_ATOM("_NET_WM_NAME");
    REGISTER_ATOM("_NET_WM_STATE");
    REGISTER_ATOM("_NET_SUPPORTING_WM_CHECK");
    REGISTER_ATOM("_NET_WM_STATE_FULLSCREEN");
    REGISTER_ATOM("_NET_WM_WINDOW_TYPE");
    REGISTER_ATOM("_NET_WM_WINDOW_TYPE_DIALOG");
    REGISTER_ATOM("_NET_CLIENT_LIST");
#undef REGISTER_ATOM
}
*/

int WindowManager::init()
{
	m_connection = xcb_connect(NULL, NULL);
	m_screen = xcb_setup_roots_iterator(xcb_get_setup(m_connection)).data;

	// XCB Exmaple from which most of my code is based : https://github.com/mchackorg/mcwm/blob/master/mcwm.c

	xcb_grab_button(m_connection,
	                true,
	                m_screen->root,
	                XCB_EVENT_MASK_BUTTON_MOTION | XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
	                XCB_GRAB_MODE_ASYNC,
	                XCB_GRAB_MODE_ASYNC,
	                XCB_NONE,
	                XCB_NONE,
	                XCB_BUTTON_INDEX_1 | XCB_BUTTON_INDEX_3,
	                XCB_MOD_MASK_ANY);

	// Get events
	{
		uint32_t values[2];
		values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

		// To get maprequest events
		auto cookie = xcb_change_window_attributes_checked(m_connection, m_screen->root, XCB_CW_EVENT_MASK, values);
		auto error = xcb_request_check(m_connection, cookie);

		xcb_flush(m_connection);

		if (error) {
			fprintf(stderr,
			        "zwm: Can't get substructure redirect. Is another WM running?\nError code: %d\n",
			        error->error_code);
			xcb_disconnect(m_connection);
			return -1;
		}
	}

	// Get existing windows to frame them
	reparent_existing_windows();

	// Set cursor
	{
		xcb_cursor_context_t *ctx;
		if (xcb_cursor_context_new(m_connection, m_screen, &ctx) >= 0) {
			auto cursor = xcb_cursor_load_cursor(ctx, "left_ptr");
			if (cursor != XCB_CURSOR_NONE) {
				fprintf(stdout, "Setting root cursor\n");
				xcb_change_window_attributes(m_connection, m_screen->root, XCB_CW_CURSOR, &cursor);
			}
			xcb_cursor_context_free(ctx);
		}
	}

	return 0;
}

void WindowManager::reparent_existing_windows()
{
	fprintf(stdout, "Reparenting windows\n");
	// Get window tree
	auto tree = xcb_query_tree(m_connection, m_screen->root);
	auto *tree_reply = xcb_query_tree_reply(m_connection, tree, 0);

	if (!tree_reply) {
		return;
	}

	auto len = xcb_query_tree_children_length(tree_reply);
	xcb_window_t *children = xcb_query_tree_children(tree_reply);
	fprintf(stdout, "Found %d children\n", len);

	for (int i = 0; i < len; i++) {
		auto attributes = xcb_get_window_attributes(m_connection, children[i]);
		auto *attributes_reply = xcb_get_window_attributes_reply(m_connection, attributes, 0);

		if (!attributes_reply) {
			fprintf(stderr, "Couldn't get attributes fow window 0x%x\n", children[i]);
			continue;
		}

		// Ignore window that shouldn't be reported to us, or that are invisible.
		if (!attributes_reply->override_redirect && attributes_reply->map_state == XCB_MAP_STATE_VIEWABLE) {
			fprintf(stdout, "Framing 0x%x\n", children[i]);
			auto *framed_window = new ZWM::FramedWindow(m_connection, m_screen, children[i], true);
			m_frames_to_framedwindows[framed_window->frame()] = framed_window;
		}
	}
}

void WindowManager::run_loop()
{
	// Loop initialization

	ZWM::Position last_cursor_position{};
	xcb_generic_event_t *event;

	fprintf(stdout, "ZWM initialized! Starting event loop\n");

	// Loop

	while (true) {
		// TODO: Use xcb_poll_for_event(conn);
		event = xcb_wait_for_event(m_connection);

		if (!event) {
			fprintf(stderr, "Event error!\n");
			if (xcb_connection_has_error(m_connection)) {
				fprintf(stderr, "Connection error, I give up\n");
				exit(-1);
			}
			continue;
		}

		switch (event->response_type) {
		case XCB_BUTTON_PRESS: {
			auto e = (xcb_button_press_event_t *)event;
			fprintf(stdout, "Button press event\n");
			// TODO: Reimplement button press.

			/*
			if (event.xbutton.subwindow || event.xbutton.window != event.xbutton.root) // clicked on a window or a
			window's frame
			{
			    Window window;
			    if (event.xbutton.subwindow)
			        window = event.xbutton.subwindow;
			    else
			        window = event.xbutton.window;

			    last_cursor_position = {event.xbutton.x_root, event.xbutton.y_root};

			    XRaiseWindow(m_display, window);
			    XSetInputFocus(m_display, window, RevertToParent, CurrentTime);
			}
			*/
			break;
		}

		case XCB_MOTION_NOTIFY: {
			auto e = (xcb_motion_notify_event_t *)event;
			fprintf(stdout, "Motion notify event\n");
			// TODO: Reimplement motion notify.

			/*
			ZWM::Position current_cursor_position{event.xbutton.x_root, event.xbutton.y_root};
			int xdiff = current_cursor_position.x - last_cursor_position.x;
			int ydiff = current_cursor_position.y - last_cursor_position.y;

			if (event.xbutton.subwindow)
			{
			    // The cursor is on this window. This should actually be the window's frame.
			    auto window = event.xbutton.subwindow;

			    if (!m_frames_to_framedwindows.count(window))
			    {
			        fprintf(stderr, "ERR: Motion on invalid window!\n");
			        break;
			    }

			    auto framed_window = m_frames_to_framedwindows[window];
			    if (event.xbutton.state & Mod1Mask | Mod2Mask) // If ALT is pressed
			    {											   // We handle keybinding for position-agnostic actions

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
			        if (!m_frames_to_framedwindows.count(window))
			        {
			            fprintf(stderr, "ERR: The frame is not a frame. How did we get here?!\n");
			            break;
			        }
			        auto framed_window = m_frames_to_framedwindows[window];

			        ZWM::Position new_pos = framed_window->pos();
			        new_pos.x += xdiff;
			        new_pos.y += ydiff;
			        framed_window->move(new_pos);
			    }
			}

			last_cursor_position = current_cursor_position;
			*/
			break;
		}

		case XCB_BUTTON_RELEASE: {
			auto e = (xcb_button_release_event_t *)event;
			fprintf(stdout, "Button release event\n");
			// TODO: Reimplement button release.

			// last_cursor_position = ZWM::Position{};
			break;
		}

		case XCB_MAP_REQUEST: {
			auto e = (xcb_map_request_event_t *)event;

			xcb_map_window(m_connection, e->window);
			auto *framed_window = new ZWM::FramedWindow(m_connection, m_screen, e->window, false);
			m_frames_to_framedwindows[framed_window->frame()] = framed_window;
			break;
		}

		case XCB_UNMAP_NOTIFY: {
			auto e = (xcb_unmap_notify_event_t *)event;

			auto frame = find_frame_for_xwindow(e->window);
			if (frame) {
				auto framed_window = m_frames_to_framedwindows[frame];
				m_frames_to_framedwindows.erase(frame);
				delete framed_window;
			}
			break;
		}

		case XCB_CONFIGURE_REQUEST: {
			auto e = (xcb_configure_request_event_t *)event;

			auto window = e->window;
			auto frame = find_frame_for_xwindow(window);
			if (frame) {
				auto framed_window = m_frames_to_framedwindows[frame];
				framed_window->move({e->x, e->y});
				framed_window->resize({e->width, e->height});
			} else {

				// Pretty much stolen from i3wm
				// We have to do this to reconstruct the values to pass to xcb_configure_window()
				// as this isn't directly given to us by the XCB API.
				//
				// TODO: Cleanup ? There may be an easier way to do this in C++

				uint32_t final_mask;
				uint32_t final_values[7];
				unsigned int i;

#define COPY_VALUE_IF_MASK_SET(event_mask, event_value)                                                                \
	if (e->value_mask & event_mask) {                                                                                  \
		final_mask |= event_mask;                                                                                      \
		final_values[i++] = e->event_value;                                                                            \
	}
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_X, x);
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_Y, y);
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_WIDTH, width);
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_HEIGHT, height);
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_BORDER_WIDTH, border_width);
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_SIBLING, sibling);
				COPY_VALUE_IF_MASK_SET(XCB_CONFIG_WINDOW_STACK_MODE, stack_mode);
#undef COPY_VALUE_IF_MASK_SET

				// Now we call xcb_configure_window() with the correct values.
				auto cookie = xcb_configure_window(m_connection, window, final_mask, final_values);
				xcb_flush(m_connection);
			}
			break;
		}

			// Events to ignore.
			/*
			case CreateNotify:
			case DestroyNotify:
			case ConfigureNotify:
			case MapNotify:
			case ReparentNotify:
			    break;
			    */
		case XCB_EXPOSE: {
			// When a window is exposed
			// (we want to draw it)
			auto e = (xcb_expose_event_t *)event;

			if (!m_frames_to_framedwindows.contains(e->window)) continue; // If the window is not a frame, we ignore it.
			auto framed_window = m_frames_to_framedwindows[e->window];

			// We draw the frame
			framed_window->draw();
		}

		default:
			fprintf(stdout, "Unhandled event %d\n", event->response_type);
			break;
		}
	}
}
} // namespace ZWM
