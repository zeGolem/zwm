#include "FramedWindow.h"

#include <stdio.h>
#include <string>

namespace ZWM
{
	FramedWindow::FramedWindow()
	{
		fprintf(stderr, "ERR: Trying to initialize a FramedWindow with no window. This won't work!\n");
	}

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

	FramedWindow::~FramedWindow()
	{
		fprintf(stdout, "Destroying framed window (xwindow=0x%x)\n", m_window);

		XReparentWindow(m_disp, m_window, DefaultRootWindow(m_disp),
						m_pos.x, m_pos.y);

		XUnmapWindow(m_disp, m_frame);
		XRemoveFromSaveSet(m_disp, m_window);
		XDestroyWindow(m_disp, m_frame);
	}

	void FramedWindow::move(Position new_pos)
	{
		XMoveWindow(m_disp, m_frame, new_pos.x, new_pos.y);
		m_pos = new_pos;
		redraw_title();
	}

	void FramedWindow::resize(Size new_size)
	{
		if (new_size.width < 1 || new_size.height - TOPBAR_HEIGHT < 1)
			return;
		XResizeWindow(m_disp, m_frame, new_size.width, new_size.height);
		XResizeWindow(m_disp, m_window, new_size.width, new_size.height - TOPBAR_HEIGHT);
		m_size = new_size;
		redraw_title();
	}

	void FramedWindow::draw_text(std::string text, Position position)
	{
		auto black = BlackPixel(m_disp, DefaultScreen(m_disp));
		auto white = WhitePixel(m_disp, DefaultScreen(m_disp));
		auto gc = XCreateGC(m_disp, m_frame, 0, 0);

		XSetForeground(m_disp, gc, black);
		XSetBackground(m_disp, gc, black);

		auto font = XLoadQueryFont(m_disp, "fixed");
		XSetFont(m_disp, gc, font->fid);
		XClearWindow(m_disp, m_frame);
		XDrawString(m_disp, m_frame, gc, position.x, position.y, text.c_str(), text.length());
	}

	void FramedWindow::draw_image(char *data, Position pos, Size size)
	{
		// Copied from http://users.polytech.unice.fr/~buffa/cours/X11_Motif/cours/XlibImages.html
		auto img = XCreateImage(m_disp, DefaultVisual(m_disp, 0), 8, ZPixmap, 0, data, size.width, size.height, 8, 0);
		auto gc = XCreateGC(m_disp, m_frame, 0, 0);
		XPutImage(m_disp, m_frame, gc, img, 0, 0, pos.x, pos.y, size.width, size.height);
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
