#pragma once

// Platform detection macros
#if defined(_WIN32) || defined(_WIN64)
    #ifndef PLATFORM_WINDOWS
        #define PLATFORM_WINDOWS 1
    #endif
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #ifndef PLATFORM_IOS
            #define PLATFORM_IOS 1
        #endif
    #else
        #ifndef PLATFORM_MACOS
            #define PLATFORM_MACOS 1
        #endif
    #endif
#elif defined(__ANDROID__)
    #ifndef PLATFORM_ANDROID
        #define PLATFORM_ANDROID 1
    #endif
#elif defined(__linux__)
    #ifndef PLATFORM_LINUX
        #define PLATFORM_LINUX 1
    #endif
#elif defined(__EMSCRIPTEN__)
    #ifndef PLATFORM_WEB
        #define PLATFORM_WEB 1
    #endif
#endif

// Desktop platforms
#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_MACOS) || defined(PLATFORM_LINUX)
    #define PLATFORM_DESKTOP 1
#endif

// Mobile platforms
#if defined(PLATFORM_IOS) || defined(PLATFORM_ANDROID)
    #define PLATFORM_MOBILE 1
#endif

// Export/Import macros for shared library
#if defined(PLATFORM_WINDOWS)
    #ifdef TOYFRAMEV_EXPORTS
        #define TOYFRAMEV_API __declspec(dllexport)
    #else
        #define TOYFRAMEV_API __declspec(dllimport)
    #endif
#else
    #define TOYFRAMEV_API __attribute__((visibility("default")))
#endif

// Static library - no export needed
#ifndef TOYFRAMEV_API
    #define TOYFRAMEV_API
#endif
