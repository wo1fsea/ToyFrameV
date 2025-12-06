/**
 * @file PlatformiOS.mm
 * @brief iOS platform implementation
 */

#include "ToyFrameV/App.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_IOS

#import <UIKit/UIKit.h>

namespace ToyFrameV {

bool App::PlatformInit() {
    // iOS initialization handled by UIApplicationMain
    return true;
}

void App::PlatformUpdate() {
    // iOS event loop is managed by UIKit
    @autoreleasepool {
        [[NSRunLoop currentRunLoop] runMode:NSDefaultRunLoopMode
                                 beforeDate:[NSDate distantPast]];
    }
}

void App::PlatformShutdown() {
    // Cleanup iOS resources
}

} // namespace ToyFrameV

#endif // PLATFORM_IOS
