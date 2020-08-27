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

        Display *m_display;
        std::unordered_map<Window, ZWM::FramedWindow*> m_frames_to_framedwindows;

    public:
        static WindowManager *the();

        // Initializes the WM. Returns 0 if successful.
        int init();
        void run_loop();
        ~WindowManager();
    };
} // namespace ZWM
