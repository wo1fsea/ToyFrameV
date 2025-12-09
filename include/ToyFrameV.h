#pragma once

/**
 * @file ToyFrameV.h
 * @brief Main include header for ToyFrameV framework
 * 
 * This is the only header users need to include.
 * All graphics functionality is abstracted - no LLGL knowledge required.
 */

#include "ToyFrameV/App.h"
#include "ToyFrameV/Graphics.h"
#include "ToyFrameV/Input.h"
#include "ToyFrameV/KeyCodes.h"
#include "ToyFrameV/Platform.h"
#include "ToyFrameV/Window.h"

// System architecture
#include "ToyFrameV/GraphicsSystem.h"
#include "ToyFrameV/IOSystem.h"
#include "ToyFrameV/InputSystem.h"
#include "ToyFrameV/System.h"
#include "ToyFrameV/TimerSystem.h"
#include "ToyFrameV/WindowSystem.h"
#include "ToyFrameV/Core/Log.h"
#include "ToyFrameV/Core/Threading.h"

// Version info
#define TOYFRAMEV_VERSION_MAJOR 0
#define TOYFRAMEV_VERSION_MINOR 1
#define TOYFRAMEV_VERSION_PATCH 0

/**
 * @brief Main entry point macro
 * 
 * Use this macro to define the main entry point for your application.
 * Example:
 * @code
 * class MyApp : public ToyFrameV::App { ... };
 * TOYFRAMEV_MAIN(MyApp)
 * @endcode
 */
#define TOYFRAMEV_MAIN(AppClass) \
    int main() { \
        AppClass app; \
        return app.Run(); \
    }
