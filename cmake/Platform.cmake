# Platform-specific CMake configuration

# Windows-specific settings
if(PLATFORM_WINDOWS)
    # Use Unicode
    add_definitions(-DUNICODE -D_UNICODE)
    # Suppress min/max macros
    add_definitions(-DNOMINMAX)
    # Windows version targeting
    add_definitions(-D_WIN32_WINNT=0x0A00)  # Windows 10+
endif()

# macOS-specific settings
if(PLATFORM_MACOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
endif()

# iOS-specific settings
if(PLATFORM_IOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum iOS version")
    set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "13.0")
endif()

# Android-specific settings
if(PLATFORM_ANDROID)
    set(ANDROID_STL c++_shared)
    set(ANDROID_CPP_FEATURES "rtti exceptions")
endif()

# Linux-specific settings
if(PLATFORM_LINUX)
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)
endif()

# Web/Emscripten-specific settings
if(PLATFORM_WEB)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(EMSCRIPTEN_FLAGS "-s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS}")
endif()
