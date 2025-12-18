#pragma once

/**
 * @file Texture.h
 * @brief Texture class for 2D texture resources
 */

#include "ToyFrameV/Graphics/Types.h"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ToyFrameV {

class Graphics;

/**
 * @brief Texture filtering mode
 */
enum class TextureFilter {
    Nearest,  // Nearest neighbor (pixelated)
    Linear    // Bilinear filtering (smooth)
};

/**
 * @brief Texture address mode (wrapping)
 */
enum class TextureAddressMode {
    Repeat,       // Repeat texture
    Mirror,       // Mirror and repeat
    Clamp,        // Clamp to edge
    Border        // Use border color
};

/**
 * @brief Sampler description
 */
struct SamplerDesc {
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureFilter mipFilter = TextureFilter::Linear;
    TextureAddressMode addressU = TextureAddressMode::Repeat;
    TextureAddressMode addressV = TextureAddressMode::Repeat;
    TextureAddressMode addressW = TextureAddressMode::Repeat;
    float mipLODBias = 0.0f;
    uint32_t maxAnisotropy = 1;
    float borderColor[4] = {0, 0, 0, 0};
};

/**
 * @brief Texture description
 */
struct TextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    bool generateMipmaps = false;
    const void* initialData = nullptr;  // Initial pixel data (optional)
    size_t dataSize = 0;                // Size of initial data
};

/**
 * @brief Sampler object for texture sampling parameters
 */
class Sampler {
  public:
    ~Sampler();

    // Internal - for backend access
    void* GetHandle() const { return m_handle; }

  private:
    friend class Graphics;
    Sampler() = default;

    void* m_handle = nullptr;
    Graphics* m_graphics = nullptr;
};

/**
 * @brief 2D Texture resource
 *
 * Textures can be created from image files or raw pixel data.
 * Use with Sampler to control filtering and wrapping.
 *
 * Usage:
 * @code
 * // Create from raw data
 * TextureDesc desc;
 * desc.width = 256;
 * desc.height = 256;
 * desc.format = PixelFormat::RGBA8;
 * desc.initialData = pixelData;
 * desc.dataSize = 256 * 256 * 4;
 * auto texture = graphics->CreateTexture(desc);
 *
 * // Load from file using AssetSystem
 * auto texture = assetSystem->LoadTexture("assets://textures/logo.png");
 *
 * // Use in rendering
 * graphics->SetTexture(0, texture.get());
 * graphics->SetSampler(0, sampler.get());
 * @endcode
 */
class Texture {
  public:
    ~Texture();

    /**
     * @brief Get width in pixels
     */
    uint32_t GetWidth() const { return m_width; }

    /**
     * @brief Get height in pixels
     */
    uint32_t GetHeight() const { return m_height; }

    /**
     * @brief Get pixel format
     */
    PixelFormat GetFormat() const { return m_format; }

    /**
     * @brief Check if texture has mipmaps
     */
    bool HasMipmaps() const { return m_hasMipmaps; }

    // Internal - for backend access
    void* GetHandle() const { return m_handle; }

  private:
    friend class Graphics;
    Texture() = default;

    void* m_handle = nullptr;
    Graphics* m_graphics = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    PixelFormat m_format = PixelFormat::RGBA8;
    bool m_hasMipmaps = false;
};

/**
 * @brief Image data loaded from file
 *
 * Raw pixel data that can be used to create textures.
 */
struct ImageData {
    std::vector<uint8_t> pixels;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t channels = 0;  // 1=R, 2=RG, 3=RGB, 4=RGBA

    bool IsValid() const { return !pixels.empty() && width > 0 && height > 0; }

    /**
     * @brief Get pixel format based on channel count
     */
    PixelFormat GetFormat() const {
        switch (channels) {
            case 1:
                return PixelFormat::R8;
            case 2:
                return PixelFormat::RG8;
            case 3:
                return PixelFormat::RGB8;
            case 4:
            default:
                return PixelFormat::RGBA8;
        }
    }

    /**
     * @brief Load image from file
     * @param path Path to image file (PNG, JPG, BMP, etc.)
     * @return ImageData with pixel data, empty if failed
     */
    static ImageData LoadFromFile(const std::string& path);

    /**
     * @brief Load image from memory
     * @param data Image file data (PNG, JPG, etc.)
     * @param size Size of data in bytes
     * @return ImageData with pixel data, empty if failed
     */
    static ImageData LoadFromMemory(const void* data, size_t size);
};

}  // namespace ToyFrameV
