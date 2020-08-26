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
		static const unsigned int BORDER_WIDTH = 3;
		static const unsigned long BORDER_COLOR = 0x037ffc;
		static const unsigned long BG_COLOR = 0x4074a8;

		Window m_window;
		Frame m_frame;
		Position m_pos;
		Size m_size;
		Display *m_disp;

	public:
		FramedWindow();
		FramedWindow(Display *, Window framed_window);
		~FramedWindow();

		void move(Position);
		void resize(Size);

		Window framed_window() const { return m_window; }
		Frame frame() const { return m_frame; }
		Position pos() const { return m_pos; }
		Size size() const { return m_size; }
	};
} // namespace ZWM
