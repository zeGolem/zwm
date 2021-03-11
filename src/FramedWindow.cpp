#include "FramedWindow.h"
#include "WindowManager.h"

#include <bits/stdint-uintn.h>
#include <stdio.h>
#include <string>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

namespace ZWM
{
	FramedWindow::FramedWindow()
	{
		fprintf(stderr, "ERR: Trying to initialize a FramedWindow with no window. This won't work!\n");
	}

	FramedWindow::FramedWindow(
			xcb_connection_t *conn,
			xcb_screen_t *screen, 
			xcb_window_t window,
			xcb_get_window_attributes_reply_t *attributes,
			bool top_bar
		)
		: m_screen(screen), m_window(window), m_connection(conn), m_has_top_bar(top_bar)
	{
		// Get window geometry
		auto window_geometry = xcb_get_geometry_reply(m_connection, xcb_get_geometry(m_connection, m_window), 0);
		m_pos = { static_cast<unsigned int>(window_geometry->x), static_cast<unsigned int>(window_geometry->y) };
		m_size = { window_geometry->width, window_geometry->height };

		// Create frame
		xcb_window_t frame_id = xcb_generate_id(m_connection);
		xcb_create_window_checked(
				m_connection, XCB_COPY_FROM_PARENT, frame_id, m_screen->root,
				m_pos.x, m_pos.y, m_size.width, m_size.height,
				BORDER_WIDTH, XCB_WINDOW_CLASS_INPUT_OUTPUT, m_screen->root_visual, 0, NULL
			);

		xcb_map_window(m_connection, frame_id);
		xcb_flush(m_connection);

		m_frame = frame_id;

		// Reparent
		xcb_reparent_window_checked(m_connection, m_window, m_frame, 0, 0 + TOPBAR_HEIGHT);

		// Recieve events from the window
		{
			uint32_t values[2];
			values[0] = XCB_EVENT_MASK_ENTER_WINDOW;
			xcb_change_window_attributes_checked(m_connection, m_window, XCB_CW_EVENT_MASK, values);
		}

		// Save the window so that it's restored if we crash.
		xcb_change_save_set(m_connection, XCB_SET_MODE_INSERT, m_window);
		xcb_flush(m_connection);

		set_title("TODO: Add title support");
		// TODO: Add title support
	}
/*
	FramedWindow::FramedWindow(Display *disp, Window window, bool has_top_bar)
		: m_window(window), m_disp(disp), m_has_top_bar(has_top_bar)
	{
		fprintf(stdout, "Creating FramedWindow for xwindow %x\n", window);

		XWindowAttributes attrs;
		if (!XGetWindowAttributes(disp, window, &attrs))
		{
			fprintf(stderr, "Could not get window attributes for %x\n", window);
			return;
		}

		if (attrs.override_redirect || attrs.map_state == IsUnviewable || attrs.map_state == IsUnmapped)
		{
			fprintf(stdout, "Skipping framing on window %x because it's invisible\n", window);
			return;
		}

		Frame frame = XCreateSimpleWindow(disp, DefaultRootWindow(disp),
										  attrs.x, attrs.y, attrs.width, attrs.height + TOPBAR_HEIGHT,
										  BORDER_WIDTH, BORDER_COLOR, BG_COLOR);

		// Get events from the frame
		XSelectInput(disp, frame, SubstructureNotifyMask | SubstructureRedirectMask | ButtonPressMask | ButtonMotionMask);

		XAddToSaveSet(disp, window);
		XReparentWindow(disp, window, frame, 0, 0 + TOPBAR_HEIGHT);
		XMapWindow(disp, frame);

		m_frame = frame;
		m_pos = {attrs.x, attrs.y};
		m_size = {attrs.width, attrs.height};

		char *window_name;
		XFetchName(m_disp, window, &window_name);
		if (window_name)
			set_title(std::string(window_name));
		
		fprintf(stdout, "Created frame for xwindow 0x%x with name '%s'\n", window, window_name);
	}
*/
	FramedWindow::~FramedWindow()
	{
		fprintf(stdout, "Destroying framed window (xwindow=0x%x)\n", m_window);

		xcb_reparent_window_checked(m_connection, m_window, m_screen->root, m_pos.x, m_pos.y);

		xcb_unmap_window_checked(m_connection, m_frame);
		xcb_change_save_set_checked(m_connection, XCB_SET_MODE_DELETE, m_window);
		xcb_destroy_window_checked(m_connection, m_frame);
	}

	void FramedWindow::move(Position new_pos)
	{
		const static uint32_t position[] { new_pos.x, new_pos.y };
		xcb_configure_window(m_connection, m_frame, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, position);
		m_pos = new_pos;
		redraw_title();
	}

	void FramedWindow::resize(Size new_size)
	{
		if (new_size.width < 1 || new_size.height - TOPBAR_HEIGHT < 1)
			return;

		const static uint32_t frame_size[] { new_size.width, new_size.height };
		xcb_configure_window(m_connection, m_frame, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, frame_size);
		const static uint32_t window_size[] { new_size.width, new_size.height - TOPBAR_HEIGHT };
		xcb_configure_window(m_connection, m_window, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, window_size);

		m_size = new_size;
		redraw_title();
	}

	void FramedWindow::draw_text(std::string text, Position position)
	{
		// TODO: Add text draw for xcb
		fprintf(stderr, "TODO: Add text draw for xcb\n");
		/*
		auto black = ZWM::WindowManager::the()->black_pixel();
		auto white = ZWM::WindowManager::the()->white_pixel();
		auto gc = XCreateGC(m_disp, m_frame, 0, 0);

		XSetForeground(m_disp, gc, black);
		XSetBackground(m_disp, gc, black);

		auto font = ZWM::WindowManager::the()->default_font();
		XSetFont(m_disp, gc, font->fid);
		XClearWindow(m_disp, m_frame);
		XDrawString(m_disp, m_frame, gc, position.x, position.y, text.c_str(), text.length());
		*/
	}

	void FramedWindow::draw_image(char *data, Position pos, Size size)
	{
		// TODO: Add iamge draw for xcb
		fprintf(stderr, "TODO: Add image draw for xcb\n");

		/*
		// Copied from http://users.polytech.unice.fr/~buffa/cours/X11_Motif/cours/XlibImages.html
		auto img = XCreateImage(m_disp, DefaultVisual(m_disp, 0), 8, ZPixmap, 0, data, size.width, size.height, 8, 0);
		auto gc = XCreateGC(m_disp, m_frame, 0, 0);
		XPutImage(m_disp, m_frame, gc, img, 0, 0, pos.x, pos.y, size.width, size.height);
		*/
	}

	void FramedWindow::set_title(std::string new_title)
	{
		m_title = new_title;
		redraw_title();
	}

	void FramedWindow::redraw_title()
	{
		draw_text(m_title, {5, TOPBAR_HEIGHT - 5});
	}
} // namespace ZWM
