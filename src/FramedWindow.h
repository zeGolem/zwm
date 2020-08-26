#pragma once
#include <X11/Xlib.h>

typedef Window Frame;

namespace ZWM
{
	struct Position
	{
		int x, y;
		Position &operator+=(const Position &rhs)
		{
			this->x += rhs.x;
			this->y += rhs.y;
			return *this;
		}
	};

	struct Size
	{
		int width, height;
		Size &operator+=(const Size &rhs)
		{
			this->width += rhs.width;
			this->height += rhs.height;
			return *this;
		}
	};

	class FramedWindow
	{
	private:
		// Frame configuration
		static const unsigned int BORDER_WIDTH = 3;
		static const unsigned long BORDER_COLOR = 0x037ffc;
		static const unsigned long BG_COLOR = 0x4074a8;

		static const unsigned int TOPBAR_HEIGHT = 15;

		Window m_window;
		Frame m_frame;
		Position m_pos;
		Size m_size;
		Display *m_disp;

		bool m_has_top_bar;

	public:
		FramedWindow();
		FramedWindow(Display *, Window framed_window, bool has_top_bar = true);
		~FramedWindow();

		void move(Position);
		void resize(Size);

		Window framed_window() const { return m_window; }
		Frame frame() const { return m_frame; }
		Position pos() const { return m_pos; }
		Size size() const { return m_size; }
		bool has_top_bar() { return m_has_top_bar; }
		unsigned int top_bar_size() { return m_has_top_bar ? TOPBAR_HEIGHT : 0; }
	};
} // namespace ZWM
