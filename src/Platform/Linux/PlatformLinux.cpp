/**
 * @file PlatformLinux.cpp
 * @brief Linux platform implementation
 */

#include "ToyFrameV/App.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_LINUX

#include <X11/Xlib.h>

namespace ToyFrameV {

static Display* s_display = nullptr;

bool App::PlatformInit() {
    // Initialize X11
    s_display = XOpenDisplay(nullptr);
    if (!s_display) {
        return false;
    }
    return true;
}

void App::PlatformUpdate() {
    if (!s_display) return;

    // Process X11 events
    while (XPending(s_display)) {
        XEvent event;
        XNextEvent(s_display, &event);
        
        // Event handling will be expanded in Stage 2/3
    }
}

void App::PlatformShutdown() {
    if (s_display) {
        XCloseDisplay(s_display);
        s_display = nullptr;
    }
}

} // namespace ToyFrameV

#endif // PLATFORM_LINUX
