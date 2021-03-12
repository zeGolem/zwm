#pragma once
#include <string>
#include <xcb/xcb.h>
#include <xcb/xproto.h>

namespace ZWM
{
struct Position {
	unsigned int x, y;
	auto operator+=(const Position &rhs)
	{
		this->x += rhs.x;
		this->y += rhs.y;
		return *this;
	}

	auto operator==(const Position &rhs) { return this->x == rhs.x && this->y == rhs.y; }
	auto operator!=(const Position &rhs) { return !(*this == rhs); }
};

struct Size {
	unsigned int width, height;
	Size &operator+=(const Size &rhs)
	{
		this->width += rhs.width;
		this->height += rhs.height;
		return *this;
	}

	auto operator==(const Size &rhs) { return this->width == rhs.width && this->height == rhs.height; }
	auto operator!=(const Size &rhs) { return !(*this == rhs); }
};

class FramedWindow
{
  private:
	// Frame configuration
	static const unsigned int BORDER_WIDTH = 1;
	static const unsigned long BORDER_COLOR = 0x037ffc;
	static const unsigned long BG_COLOR = 0x4074a8;

	static const unsigned int TOPBAR_HEIGHT = 15;

	xcb_connection_t *m_connection;
	xcb_screen_t *m_screen;

	xcb_window_t m_window;
	xcb_window_t m_frame;

	Position m_pos;
	Size m_size;
	std::string m_title;

	bool m_has_top_bar;

	void draw_text(std::string, Position);
	void draw_image(char *data, Position, Size);

  public:
	FramedWindow();
	// FramedWindow(Display *, Window framed_window, bool has_top_bar = true);
	FramedWindow(xcb_connection_t *,
	             xcb_screen_t *,
	             xcb_window_t framed_window,
	             xcb_get_window_attributes_reply_t *,
	             bool has_top_bar = true);
	~FramedWindow();

	void move(const Position);
	void resize(const Size);

	void set_title(const std::string);
	void redraw_title();
	void draw();

	xcb_window_t framed_window() const { return m_window; }
	xcb_window_t frame() const { return m_frame; }
	Position pos() const { return m_pos; }
	Size size() const { return m_size; }
	bool has_top_bar() const { return m_has_top_bar; }
	unsigned int top_bar_size() const { return m_has_top_bar ? TOPBAR_HEIGHT : 0; }
	std::string title() const { return m_title; }
};
} // namespace ZWM
