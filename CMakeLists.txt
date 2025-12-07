cmake_minimum_required(VERSION 3.20)
project(ToyFrameV VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/FS)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform detection
if(EMSCRIPTEN)
    set(PLATFORM_NAME "Web")
    add_definitions(-DPLATFORM_WEB=1)
elseif(WIN32)
    set(PLATFORM_NAME "Windows")
    add_definitions(-DPLATFORM_WINDOWS=1)
elseif(APPLE)
    if(IOS)
        set(PLATFORM_NAME "iOS")
        add_definitions(-DPLATFORM_IOS=1)
    else()
        set(PLATFORM_NAME "macOS")
        add_definitions(-DPLATFORM_MACOS=1)
    endif()
elseif(ANDROID)
    set(PLATFORM_NAME "Android")
    add_definitions(-DPLATFORM_ANDROID=1)
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
    add_definitions(-DPLATFORM_LINUX=1)
endif()

message(STATUS "Building for platform: ${PLATFORM_NAME}")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# =============================================================================
# Third-party dependencies via FetchContent
# =============================================================================
include(FetchContent)

# LLGL - Low Level Graphics Library
message(STATUS "Fetching LLGL...")
FetchContent_Declare(
    LLGL
    GIT_REPOSITORY https://github.com/LukasBanana/LLGL.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

# LLGL build options
set(LLGL_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LLGL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LLGL_BUILD_STATIC_LIB ON CACHE BOOL "" FORCE)

# Platform-specific LLGL backends
if(EMSCRIPTEN)
    set(LLGL_BUILD_RENDERER_WEBGL ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_NULL OFF CACHE BOOL "" FORCE)
elseif(WIN32)
    set(LLGL_BUILD_RENDERER_DIRECT3D11 ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_DIRECT3D12 OFF CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_OPENGL ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_VULKAN OFF CACHE BOOL "" FORCE)
elseif(APPLE)
    set(LLGL_BUILD_RENDERER_METAL ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_OPENGL ON CACHE BOOL "" FORCE)
elseif(UNIX)
    set(LLGL_BUILD_RENDERER_OPENGL ON CACHE BOOL "" FORCE)
    set(LLGL_BUILD_RENDERER_VULKAN OFF CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(LLGL)

# =============================================================================
# Core framework library
# =============================================================================
add_subdirectory(src)

# Samples
add_subdirectory(samples)
