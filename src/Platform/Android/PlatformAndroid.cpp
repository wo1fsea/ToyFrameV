/**
 * @file PlatformAndroid.cpp
 * @brief Android platform implementation
 */

#include "ToyFrameV/App.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_ANDROID

#include <android/log.h>
#include <android_native_app_glue.h>

namespace ToyFrameV {

static android_app* s_androidApp = nullptr;

bool App::PlatformInit() {
    // Android initialization - app state set from android_main
    return s_androidApp != nullptr;
}

void App::PlatformUpdate() {
    // Process Android events
    int events;
    android_poll_source* source;
    
    while (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
        if (source) {
            source->process(s_androidApp, source);
        }
        
        if (s_androidApp->destroyRequested) {
            Quit();
            return;
        }
    }
}

void App::PlatformShutdown() {
    // Cleanup Android resources
}

// Called from android_main to set the app instance
void SetAndroidApp(android_app* app) {
    s_androidApp = app;
}

} // namespace ToyFrameV

#endif // PLATFORM_ANDROID
