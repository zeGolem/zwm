#pragma once

#include "FramedWindow.h"

#include <xcb/xcb.h>
#include <unordered_map>
#include <xcb/xproto.h>

namespace ZWM
{
    class WindowManager
    {
    private:
        static WindowManager *m_instance;
        WindowManager() {}

        xcb_window_t find_frame_for_xwindow(xcb_window_t w);

		// TODO: Add support for text drawing with xcb
		/*
        ulong m_black_pixel;
        ulong m_white_pixel;
        XFontStruct* m_default_font;
		*/

        std::unordered_map<xcb_window_t, ZWM::FramedWindow*> m_frames_to_framedwindows;
		// TODO: Do atoms stuff with xcb
        // std::unordered_map<std::string, Atom> m_atoms;

		xcb_connection_t* m_connection;
		xcb_screen_t* m_screen;

		void reparent_existing_windows();

    public:
        static WindowManager *the();

		/*
        ulong black_pixel();
        ulong white_pixel();
        XFontStruct* default_font();
		*/

        // Initializes the WM. Returns 0 if successful.
        int init();
        // void init_atoms();
        void run_loop();
        ~WindowManager();
    };
} // namespace ZWM
