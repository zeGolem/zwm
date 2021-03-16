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

FramedWindow::FramedWindow(xcb_connection_t *conn, xcb_screen_t *screen, xcb_window_t window, bool top_bar)
    : m_screen(screen), m_window(window), m_connection(conn), m_has_top_bar(top_bar)
{
	// Get window geometry
	auto window_geometry = xcb_get_geometry_reply(m_connection, xcb_get_geometry(m_connection, m_window), 0);
	m_pos = {window_geometry->x, window_geometry->y};
	m_size = {window_geometry->width, window_geometry->height};

	// Make background pixmap
	m_frame_background = xcb_generate_id(m_connection);
	xcb_create_pixmap(
	    m_connection, m_screen->root_depth, m_frame_background, m_screen->root, size().width, size().height);

	// Create frame
	m_frame = xcb_generate_id(m_connection);
	{
		// Register events
		auto mask = XCB_CW_BACK_PIXMAP | XCB_CW_EVENT_MASK;
		// Background pixmap and events we want to get from the frame
		uint32_t values[] = {
		    m_frame_background,
		    XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY,
		};

		// Creates the frame's window.
		xcb_create_window_checked(m_connection,
		                          XCB_COPY_FROM_PARENT,
		                          m_frame,
		                          m_screen->root,
		                          m_pos.x,
		                          m_pos.y,
		                          size().width,
		                          size().height + TOPBAR_HEIGHT,
		                          BORDER_WIDTH,
		                          XCB_WINDOW_CLASS_INPUT_OUTPUT,
		                          m_screen->root_visual,
		                          mask,
		                          values);
	}

	xcb_map_window(m_connection, m_frame);
	xcb_flush(m_connection);

	// Reparent
	xcb_reparent_window_checked(m_connection, m_window, m_frame, 0, 0 + TOPBAR_HEIGHT);

	// Save the window so that it's restored if we crash.
	xcb_change_save_set(m_connection, XCB_SET_MODE_INSERT, m_window);
	xcb_flush(m_connection);

	set_title("TODO: Add title support");
	// TODO: Add title support
	// TODO: Draw to the frame window: https://xcb.freedesktop.org/tutorial/basicwindowsanddrawing/
}

FramedWindow::~FramedWindow()
{
	xcb_change_save_set(m_connection, XCB_SET_MODE_DELETE, m_window);
	xcb_reparent_window(m_connection, m_window, m_screen->root, m_pos.x, m_pos.y);
	xcb_unmap_window(m_connection, m_frame);
	xcb_destroy_window(m_connection, m_frame);
	xcb_flush(m_connection);
}

void FramedWindow::move(Position new_pos)
{
	// TODO: what? what to do if position is negative? xcb is weird...
	const static uint32_t position[]{static_cast<uint32_t>(new_pos.x), static_cast<uint32_t>(new_pos.y)};
	xcb_configure_window(m_connection, m_frame, XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, position);
	m_pos = new_pos;
	redraw_title();
}

void FramedWindow::resize(Size new_size)
{
	if (new_size.width < 1 || new_size.height - TOPBAR_HEIGHT < 1) return;

	const static uint32_t frame_size[]{new_size.width, new_size.height};
	xcb_configure_window(m_connection, m_frame, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, frame_size);
	const static uint32_t window_size[]{new_size.width, new_size.height - TOPBAR_HEIGHT};
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

void FramedWindow::draw() const
{
	fprintf(stdout, "draw\n");
	// Create foreground GC
	xcb_gcontext_t foreground_gc = xcb_generate_id(m_connection);
	uint32_t mask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	uint32_t values[] = {
	    m_screen->white_pixel,
	    m_screen->white_pixel,
	    0,
	};
	xcb_create_gc(m_connection, foreground_gc, m_frame, mask, values);

	// Draw background rectangle
	xcb_rectangle_t rect[] = {{0, 0, static_cast<uint16_t>(size().width), TOPBAR_HEIGHT}};
	xcb_poly_fill_rectangle(m_connection, m_frame, foreground_gc, 1, rect);

	xcb_flush(m_connection);
}
} // namespace ZWM
