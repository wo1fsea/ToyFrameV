/**
 * @file LLGLSurfaceAdapter.cpp
 * @brief Implementation of LLGL Surface adapter for ToyFrameV windows
 */

#include "LLGLSurfaceAdapter.h"
#include <LLGL/Display.h>

#ifdef PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

namespace ToyFrameV {

LLGLSurfaceAdapter::LLGLSurfaceAdapter(Window* window)
    : m_window(window)
{
}

bool LLGLSurfaceAdapter::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) {
    if (!m_window || !nativeHandle || nativeHandleSize != sizeof(LLGL::NativeHandle)) {
        return false;
    }

    auto* handle = reinterpret_cast<LLGL::NativeHandle*>(nativeHandle);
    
#ifdef PLATFORM_WINDOWS
    handle->window = static_cast<HWND>(m_window->GetNativeHandle());
#else
    // For other platforms, implement accordingly
    // Linux: handle->window = static_cast<::Window>(m_window->GetNativeHandle());
    // macOS: handle->window = static_cast<NSWindow*>(m_window->GetNativeHandle());
    return false;
#endif

    return true;
}

LLGL::Extent2D LLGLSurfaceAdapter::GetContentSize() const {
    if (!m_window) {
        return { 0, 0 };
    }
    
    return { 
        static_cast<std::uint32_t>(m_window->GetWidth()), 
        static_cast<std::uint32_t>(m_window->GetHeight()) 
    };
}

bool LLGLSurfaceAdapter::AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) {
    if (!m_window) {
        return false;
    }

    // Get current window size
    LLGL::Extent2D currentSize = GetContentSize();

    // If resolution is requested, provide current or adapt
    if (resolution) {
        if (resolution->width == 0 || resolution->height == 0) {
            *resolution = currentSize;
        } else {
            // Resize window to requested resolution
            m_window->SetSize(static_cast<int>(resolution->width), 
                             static_cast<int>(resolution->height));
        }
    }

    // Fullscreen handling - for now, just report current state
    if (fullscreen) {
        // ToyFrameV Window doesn't have fullscreen query yet, assume windowed
        *fullscreen = false;
    }

    return true;
}

LLGL::Display* LLGLSurfaceAdapter::FindResidentDisplay() const {
    // Use LLGL's display enumeration to find the display containing this window
    // For simplicity, return the primary display
    return LLGL::Display::GetPrimary();
}

} // namespace ToyFrameV
