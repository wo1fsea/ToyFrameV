/**
 * @file PlatformMacOS.mm
 * @brief macOS platform implementation
 */

#include "ToyFrameV/App.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_MACOS

#import <Cocoa/Cocoa.h>

namespace ToyFrameV {

static NSApplication* s_app = nullptr;

bool App::PlatformInit() {
    @autoreleasepool {
        // Initialize NSApplication
        s_app = [NSApplication sharedApplication];
        [s_app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [s_app finishLaunching];
    }
    return true;
}

void App::PlatformUpdate() {
    @autoreleasepool {
        // Process macOS events
        NSEvent* event;
        while ((event = [s_app nextEventMatchingMask:NSEventMaskAny
                                           untilDate:nil
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [s_app sendEvent:event];
        }
    }
}

void App::PlatformShutdown() {
    // Cleanup macOS resources
    s_app = nullptr;
}

} // namespace ToyFrameV

#endif // PLATFORM_MACOS
