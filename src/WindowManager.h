#pragma once

#include "FramedWindow.h"

#include <X11/Xlib.h>
#include <unordered_map>

namespace ZWM
{
    class WindowManager
    {
    private:
        static WindowManager *m_instance;
        WindowManager() {}

        Window find_frame_for_xwindow(Window w);

        ulong m_black_pixel;
        ulong m_white_pixel;
        XFontStruct* m_default_font;

        Display *m_display;
        std::unordered_map<Window, ZWM::FramedWindow*> m_frames_to_framedwindows;
        std::unordered_map<std::string, Atom> m_atoms;

    public:
        static WindowManager *the();

        ulong black_pixel();
        ulong white_pixel();
        XFontStruct* default_font();

        // Initializes the WM. Returns 0 if successful.
        int init();
        void init_atoms();
        void run_loop();
        ~WindowManager();
    };
} // namespace ZWM
