/**
 * @file PlatformWeb.cpp
 * @brief Web/Emscripten platform implementation
 */

#include "ToyFrameV/App.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_WEB

#include <emscripten.h>
#include <emscripten/html5.h>

namespace ToyFrameV {

bool App::PlatformInit() {
    // Emscripten initialization
    return true;
}

void App::PlatformUpdate() {
    // Web events are handled via callbacks registered in PlatformInit
    // The main loop is managed by emscripten_set_main_loop
}

void App::PlatformShutdown() {
    // Cleanup web resources
}

} // namespace ToyFrameV

#endif // PLATFORM_WEB
