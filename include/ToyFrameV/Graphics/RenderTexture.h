#pragma once

/**
 * @file RenderTexture.h
 * @brief Offscreen render target with readback support
 */

#include "ToyFrameV/Graphics/Types.h"
#include <functional>
#include <memory>
#include <vector>
#include <string>

namespace ToyFrameV {

class Graphics;
class RenderTextureImpl;

/**
 * @brief RenderTexture description
 */
struct RenderTextureDesc {
    uint32_t width = 256;
    uint32_t height = 256;
    PixelFormat format = PixelFormat::RGBA8;
    bool hasDepth = true;
};

/**
 * @brief Pixel data container for readback
 */
struct PixelData {
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    
    bool IsValid() const { return !data.empty() && width > 0 && height > 0; }
    
    /**
     * @brief Save to BMP file
     * @param filename Output file path
     * @return true on success
     */
    bool SaveToBMP(const std::string& filename) const;
};

/**
 * @brief Callback type for async readback
 */
using ReadbackCallback = std::function<void(PixelData)>;

/**
 * @brief Offscreen render texture
 * 
 * Allows rendering to a texture instead of the screen, with support
 * for reading back pixel data to CPU memory.
 * 
 * Usage:
 * @code
 * auto rt = graphics->CreateRenderTexture({512, 512});
 * graphics->SetRenderTarget(rt.get());
 * graphics->Clear(Color::Blue());
 * // ... draw commands ...
 * graphics->SetRenderTarget(nullptr);  // Back to screen
 * 
 * // Sync readback (blocking)
 * PixelData pixels = rt->ReadPixels();
 * pixels.SaveToBMP("screenshot.bmp");
 * 
 * // Async readback (non-blocking, recommended for WebGL)
 * rt->ReadPixelsAsync([](PixelData pixels) {
 *     pixels.SaveToBMP("screenshot.bmp");
 * });
 * @endcode
 */
class RenderTexture {
public:
    ~RenderTexture();

    /**
     * @brief Get width in pixels
     */
    uint32_t GetWidth() const;

    /**
     * @brief Get height in pixels
     */
    uint32_t GetHeight() const;

    /**
     * @brief Get pixel format
     */
    PixelFormat GetFormat() const;

    /**
     * @brief Resize the render texture
     * @note This invalidates any pending async readback
     */
    void Resize(uint32_t width, uint32_t height);

    /**
     * @brief Read pixels synchronously (blocking)
     * @return Pixel data, empty if failed
     * @note On WebGL this may cause a GPU stall
     */
    PixelData ReadPixels();

    /**
     * @brief Read pixels asynchronously (non-blocking)
     * @param callback Called when readback completes
     * @note Call Graphics::ProcessReadbacks() each frame to dispatch callbacks
     */
    void ReadPixelsAsync(ReadbackCallback callback);

    /**
     * @brief Check if async readback is pending
     */
    bool IsReadbackPending() const;

    /**
     * @brief Cancel pending async readback
     */
    void CancelReadback();

    // Internal
    RenderTextureImpl* GetImpl() const { return m_impl.get(); }

private:
    friend class Graphics;
    RenderTexture(Graphics* graphics);
    bool Initialize(const RenderTextureDesc& desc);

    std::unique_ptr<RenderTextureImpl> m_impl;
    Graphics* m_graphics = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    PixelFormat m_format = PixelFormat::RGBA8;
};

} // namespace ToyFrameV
