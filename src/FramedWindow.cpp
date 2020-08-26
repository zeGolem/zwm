#include "FramedWindow.h"

#include <stdio.h>

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

		// if (attrs.override_redirect || attrs.map_state != IsViewable)
		// {
		// 	fprintf(stdout, "Skipping framing on window %x because it's invisible\n", window);
		// 	return;
		// }

		Frame frame = XCreateSimpleWindow(disp, DefaultRootWindow(disp),
										  attrs.x, attrs.y + TOPBAR_HEIGHT, attrs.width, attrs.height + TOPBAR_HEIGHT,
										  BORDER_WIDTH, BORDER_COLOR, BG_COLOR);

		XSelectInput(disp, frame, SubstructureNotifyMask | SubstructureRedirectMask);
		XAddToSaveSet(disp, window);
		XReparentWindow(disp, window, frame, 0, 0 + TOPBAR_HEIGHT);
		XMapWindow(disp, frame);

		m_frame = frame;
		m_pos = {attrs.x, attrs.y};
		m_size = {attrs.width, attrs.height};
	}

	FramedWindow::~FramedWindow()
	{
		fprintf(stdout, "Destroying framed window\n");
		XUnmapWindow(m_disp, m_frame);

		XReparentWindow(m_disp, m_window, DefaultRootWindow(m_disp),
						m_pos.x, m_pos.y);

		XRemoveFromSaveSet(m_disp, m_window);

		XDestroyWindow(m_disp, m_frame);
	}

	void FramedWindow::move(Position new_pos)
	{
		XMoveWindow(m_disp, m_frame, new_pos.x, new_pos.y);
		m_pos = new_pos;
	}

	void FramedWindow::resize(Size new_size)
	{
		XResizeWindow(m_disp, m_frame, new_size.width, new_size.height);
		XResizeWindow(m_disp, m_window, new_size.width, new_size.height - TOPBAR_HEIGHT);
		m_size = new_size;
	}
} // namespace ZWM
