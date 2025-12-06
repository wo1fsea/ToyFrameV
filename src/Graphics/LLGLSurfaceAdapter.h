/**
 * @file LLGLSurfaceAdapter.h
 * @brief Adapter to wrap ToyFrameV::Window as LLGL::Surface
 * 
 * This allows LLGL to use an externally created window from ToyFrameV.
 */

#pragma once

#include "ToyFrameV/Window.h"
#include <LLGL/Surface.h>
#include <LLGL/Platform/NativeHandle.h>
#include <memory>

namespace ToyFrameV {

/**
 * @brief Adapter class that wraps a ToyFrameV::Window as an LLGL::Surface
 * 
 * This enables LLGL's SwapChain to render to a window created and managed
 * by ToyFrameV, rather than creating its own window.
 */
class LLGLSurfaceAdapter : public LLGL::Surface {
public:
    /**
     * @brief Create an adapter for the given ToyFrameV window
     * @param window The ToyFrameV window to wrap (must outlive this adapter)
     */
    explicit LLGLSurfaceAdapter(Window* window);
    ~LLGLSurfaceAdapter() override = default;

    // Non-copyable
    LLGLSurfaceAdapter(const LLGLSurfaceAdapter&) = delete;
    LLGLSurfaceAdapter& operator=(const LLGLSurfaceAdapter&) = delete;

    // ==================== LLGL::Surface Interface ====================

    /**
     * @brief Get the native window handle
     */
    bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) override;

    /**
     * @brief Get the content size in pixels
     */
    LLGL::Extent2D GetContentSize() const override;

    /**
     * @brief Adapt surface for video mode change
     */
    bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) override;

    /**
     * @brief Find the display this surface is on
     */
    LLGL::Display* FindResidentDisplay() const override;

    // ==================== ToyFrameV Integration ====================

    /**
     * @brief Get the underlying ToyFrameV window
     */
    Window* GetWindow() const { return m_window; }

private:
    Window* m_window = nullptr;
};

/**
 * @brief Create a shared pointer to an LLGLSurfaceAdapter
 * @param window The ToyFrameV window to wrap
 * @return Shared pointer suitable for passing to LLGL::RenderSystem::CreateSwapChain
 */
inline std::shared_ptr<LLGL::Surface> CreateLLGLSurface(Window* window) {
    return std::make_shared<LLGLSurfaceAdapter>(window);
}

} // namespace ToyFrameV
