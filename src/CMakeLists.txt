# ToyFrameV Core Library

set(CORE_SOURCES
    App.cpp
    Input.cpp
    Core/Threading.cpp
    Core/Log.cpp
    Graphics/Graphics.cpp
    System/SystemManager.cpp
    System/WindowSystem.cpp
    System/GraphicsSystem.cpp
    System/InputSystem.cpp
    System/IOSystem.cpp
)

# Platform-specific window and input sources
# Note: EMSCRIPTEN must be checked first as it also defines UNIX
if(EMSCRIPTEN)
    list(APPEND CORE_SOURCES
        Platform/Web/PlatformWeb.cpp
    )
    # WebGL doesn't use LLGLSurfaceAdapter - LLGL manages canvas directly
elseif(WIN32)
    list(APPEND CORE_SOURCES
        Platform/Windows/WindowWindows.cpp
        Platform/Windows/InputWindows.cpp
        Graphics/LLGLSurfaceAdapter.cpp
    )
elseif(APPLE)
    if(IOS)
        list(APPEND CORE_SOURCES
            Platform/iOS/WindowiOS.mm
            Platform/iOS/InputiOS.mm
        )
    else()
        list(APPEND CORE_SOURCES
            Platform/macOS/WindowMacOS.mm
            Platform/macOS/InputMacOS.mm
        )
    endif()
elseif(ANDROID)
    list(APPEND CORE_SOURCES
        Platform/Android/WindowAndroid.cpp
        Platform/Android/InputAndroid.cpp
    )
elseif(UNIX)
    list(APPEND CORE_SOURCES
        Platform/Linux/WindowLinux.cpp
        Platform/Linux/InputLinux.cpp
        Graphics/LLGLSurfaceAdapter.cpp
    )
endif()

# Create library
add_library(ToyFrameV STATIC ${CORE_SOURCES})

# Include directories
target_include_directories(ToyFrameV PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party
    ${llgl_SOURCE_DIR}/include
)

# Link platform-specific renderers FIRST
if(EMSCRIPTEN)
    # WebGL uses LLGL_WebGL (if available) or built-in WebGL support
    if(TARGET LLGL_WebGL)
        target_link_libraries(ToyFrameV PUBLIC LLGL_WebGL)
    endif()
elseif(WIN32)
    target_link_libraries(ToyFrameV PUBLIC
        LLGL_OpenGL
        LLGL_Direct3D11
        LLGL_DXCommon
        LLGL_Null
    )
elseif(APPLE)
    target_link_libraries(ToyFrameV PUBLIC
        LLGL_Metal
    )
else()
    # Force link all symbols from LLGL modules on Linux
    target_link_libraries(ToyFrameV PUBLIC
        -Wl,--whole-archive
        LLGL_OpenGL
        LLGL_Null
        -Wl,--no-whole-archive
    )
endif()

# Link LLGL core AFTER modules
target_link_libraries(ToyFrameV PUBLIC LLGL)

# Platform-specific link libraries
if(EMSCRIPTEN)
    # Emscripten WebGL2 settings - must be PUBLIC so they propagate to executables
    target_compile_options(ToyFrameV PUBLIC -sUSE_WEBGL2=1)
    target_link_options(ToyFrameV PUBLIC
        -sUSE_WEBGL2=1
        -sFULL_ES3=1
        -sMIN_WEBGL_VERSION=2
        -sMAX_WEBGL_VERSION=2
    )
elseif(WIN32)
    target_link_libraries(ToyFrameV PRIVATE
        user32
        gdi32
        d3d11
        d3dcompiler
        dxgi
        opengl32
    )
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(METAL_LIBRARY Metal)
    find_library(QUARTZCORE_LIBRARY QuartzCore)
    target_link_libraries(ToyFrameV PRIVATE
        ${COCOA_LIBRARY}
        ${METAL_LIBRARY}
        ${QUARTZCORE_LIBRARY}
    )
elseif(UNIX AND NOT ANDROID)
    find_package(X11 REQUIRED)
    target_link_libraries(ToyFrameV PRIVATE
        ${X11_LIBRARIES}
        pthread
    )
endif()
