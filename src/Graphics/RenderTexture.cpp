/**
 * @file RenderTexture.cpp
 * @brief RenderTexture implementation using LLGL
 */

#include "ToyFrameV/Graphics/RenderTexture.h"
#include "ToyFrameV/Graphics/Context.h"

#include <LLGL/LLGL.h>
#include <fstream>
#include <cstring>
#include <queue>
#include <mutex>

namespace ToyFrameV {

// Forward declaration - defined in Graphics.cpp
class GraphicsImpl;

// External function to get LLGL render system (defined in Graphics.cpp)
extern LLGL::RenderSystem* GetLLGLRenderSystem(GraphicsImpl* impl);

// Forward declaration of RenderTextureImpl for helper function
class RenderTextureImpl;

// ============================================================================
// PixelData - BMP Export
// ============================================================================

bool PixelData::SaveToBMP(const std::string& filename) const {
    if (!IsValid()) return false;
    if (format != PixelFormat::RGBA8 && format != PixelFormat::BGRA8) {
        return false;  // Only support 32-bit formats for now
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;

    // BMP Header (14 bytes)
    uint32_t rowSize = ((width * 4 + 3) / 4) * 4;  // Rows padded to 4-byte boundary
    uint32_t pixelDataSize = rowSize * height;
    uint32_t fileSize = 54 + pixelDataSize;
    
    uint8_t bmpHeader[14] = {
        'B', 'M',                           // Signature
        0, 0, 0, 0,                          // File size (filled below)
        0, 0, 0, 0,                          // Reserved
        54, 0, 0, 0                          // Data offset
    };
    std::memcpy(&bmpHeader[2], &fileSize, 4);

    // DIB Header (40 bytes) - BITMAPINFOHEADER
    uint8_t dibHeader[40] = {0};
    dibHeader[0] = 40;                       // Header size
    std::memcpy(&dibHeader[4], &width, 4);   // Width
    int32_t negHeight = -static_cast<int32_t>(height);  // Negative for top-down
    std::memcpy(&dibHeader[8], &negHeight, 4);  // Height (negative = top-down)
    dibHeader[12] = 1;                       // Color planes
    dibHeader[14] = 32;                      // Bits per pixel
    dibHeader[16] = 0;                       // No compression

    file.write(reinterpret_cast<char*>(bmpHeader), 14);
    file.write(reinterpret_cast<char*>(dibHeader), 40);

    // Write pixel data (BMP uses BGRA order)
    if (format == PixelFormat::RGBA8) {
        // Convert RGBA to BGRA
        std::vector<uint8_t> bgraData(width * height * 4);
        for (size_t i = 0; i < data.size(); i += 4) {
            bgraData[i + 0] = data[i + 2];  // B
            bgraData[i + 1] = data[i + 1];  // G
            bgraData[i + 2] = data[i + 0];  // R
            bgraData[i + 3] = data[i + 3];  // A
        }
        // Write rows (BMP may need padding)
        for (uint32_t y = 0; y < height; ++y) {
            file.write(reinterpret_cast<char*>(&bgraData[y * width * 4]), width * 4);
            // Padding to 4-byte boundary (not needed for 32-bit but good practice)
            uint32_t padding = rowSize - width * 4;
            if (padding > 0) {
                uint8_t pad[4] = {0};
                file.write(reinterpret_cast<char*>(pad), padding);
            }
        }
    } else {
        // Already BGRA
        for (uint32_t y = 0; y < height; ++y) {
            file.write(reinterpret_cast<const char*>(&data[y * width * 4]), width * 4);
            uint32_t padding = rowSize - width * 4;
            if (padding > 0) {
                uint8_t pad[4] = {0};
                file.write(reinterpret_cast<char*>(pad), padding);
            }
        }
    }

    return true;
}

// ============================================================================
// RenderTextureImpl
// ============================================================================

class RenderTextureImpl {
public:
    LLGL::Texture* colorTexture = nullptr;
    LLGL::Texture* depthTexture = nullptr;
    LLGL::RenderTarget* renderTarget = nullptr;
    LLGL::RenderSystem* renderSystem = nullptr;
    
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    bool hasDepth = true;

    // Async readback state
    struct ReadbackRequest {
        ReadbackCallback callback;
        LLGL::Buffer* stagingBuffer = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        PixelFormat format = PixelFormat::RGBA8;
        bool copyIssued = false;
    };
    std::queue<ReadbackRequest> pendingReadbacks;
    mutable std::mutex readbackMutex;

    ~RenderTextureImpl() {
        Cleanup();
    }

    void Cleanup() {
        if (renderSystem) {
            // Cancel pending readbacks
            std::lock_guard<std::mutex> lock(readbackMutex);
            while (!pendingReadbacks.empty()) {
                auto& req = pendingReadbacks.front();
                if (req.stagingBuffer) {
                    renderSystem->Release(*req.stagingBuffer);
                }
                pendingReadbacks.pop();
            }

            if (renderTarget) {
                renderSystem->Release(*renderTarget);
                renderTarget = nullptr;
            }
            if (colorTexture) {
                renderSystem->Release(*colorTexture);
                colorTexture = nullptr;
            }
            if (depthTexture) {
                renderSystem->Release(*depthTexture);
                depthTexture = nullptr;
            }
        }
    }
    
    bool Create(LLGL::RenderSystem* rs, uint32_t w, uint32_t h, PixelFormat fmt, bool depth) {
        renderSystem = rs;
        width = w;
        height = h;
        format = fmt;
        hasDepth = depth;
        
        // Create color texture
        LLGL::TextureDescriptor texDesc;
        texDesc.type = LLGL::TextureType::Texture2D;
        texDesc.extent = { width, height, 1 };
        texDesc.mipLevels = 1;
        texDesc.bindFlags = LLGL::BindFlags::Sampled | 
                            LLGL::BindFlags::ColorAttachment |
                            LLGL::BindFlags::CopySrc;

        switch (format) {
            case PixelFormat::RGBA8:
                texDesc.format = LLGL::Format::RGBA8UNorm;
                break;
            case PixelFormat::BGRA8:
                texDesc.format = LLGL::Format::BGRA8UNorm;
                break;
            case PixelFormat::RGBA16F:
                texDesc.format = LLGL::Format::RGBA16Float;
                break;
            case PixelFormat::RGBA32F:
                texDesc.format = LLGL::Format::RGBA32Float;
                break;
            default:
                texDesc.format = LLGL::Format::RGBA8UNorm;
                break;
        }

        colorTexture = renderSystem->CreateTexture(texDesc);
        if (!colorTexture) return false;

        // Create depth texture if needed
        if (hasDepth) {
            LLGL::TextureDescriptor depthDesc;
            depthDesc.type = LLGL::TextureType::Texture2D;
            depthDesc.extent = { width, height, 1 };
            depthDesc.mipLevels = 1;
            depthDesc.format = LLGL::Format::D32Float;
            depthDesc.bindFlags = LLGL::BindFlags::DepthStencilAttachment;

            depthTexture = renderSystem->CreateTexture(depthDesc);
        }

        // Create render target
        LLGL::RenderTargetDescriptor rtDesc;
        rtDesc.resolution = { width, height };
        rtDesc.colorAttachments[0] = colorTexture;
        if (depthTexture) {
            rtDesc.depthStencilAttachment = depthTexture;
        }

        renderTarget = renderSystem->CreateRenderTarget(rtDesc);
        return renderTarget != nullptr;
    }
};

// Helper function for Graphics.cpp to access LLGL RenderTarget
LLGL::RenderTarget* GetLLGLRenderTarget(RenderTextureImpl* impl) {
    return impl ? impl->renderTarget : nullptr;
}

// ============================================================================
// RenderTexture Implementation
// ============================================================================

RenderTexture::RenderTexture(Graphics* graphics)
    : m_impl(std::make_unique<RenderTextureImpl>())
    , m_graphics(graphics)
{
}

RenderTexture::~RenderTexture() = default;

bool RenderTexture::Initialize(const RenderTextureDesc& desc) {
    if (!m_graphics || !m_graphics->GetImpl()) return false;

    m_width = desc.width;
    m_height = desc.height;
    m_format = desc.format;

    // Get LLGL render system from Graphics
    // Note: GraphicsImpl has a public renderSystem member
    auto* graphicsImpl = m_graphics->GetImpl();
    
    // Access the render system - defined in Graphics.cpp (ToyFrameV namespace)
    auto* renderSystem = ToyFrameV::GetLLGLRenderSystem(graphicsImpl);
    if (!renderSystem) return false;

    return m_impl->Create(renderSystem, m_width, m_height, m_format, desc.hasDepth);
}

uint32_t RenderTexture::GetWidth() const { return m_width; }
uint32_t RenderTexture::GetHeight() const { return m_height; }
PixelFormat RenderTexture::GetFormat() const { return m_format; }

void RenderTexture::Resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return;

    // Cancel pending readbacks
    CancelReadback();

    // Store old settings
    bool hasDepth = m_impl->hasDepth;
    auto* renderSystem = m_impl->renderSystem;

    // Cleanup old resources
    m_impl->Cleanup();

    // Update dimensions
    m_width = width;
    m_height = height;

    // Recreate
    m_impl->Create(renderSystem, m_width, m_height, m_format, hasDepth);
}

PixelData RenderTexture::ReadPixels() {
    PixelData result;
    if (!m_impl->colorTexture || !m_impl->renderSystem) return result;

    result.width = m_width;
    result.height = m_height;
    result.format = m_format;

    uint32_t bytesPerPixel = GetBytesPerPixel(m_format);
    result.data.resize(m_width * m_height * bytesPerPixel);

    // Use LLGL ReadTexture for synchronous readback
    LLGL::MutableImageView dstImage;
    dstImage.format = LLGL::ImageFormat::RGBA;
    dstImage.dataType = LLGL::DataType::UInt8;
    dstImage.data = result.data.data();
    dstImage.dataSize = result.data.size();

    LLGL::TextureRegion region;
    region.subresource = { 0, 1, 0, 1 };
    region.offset = { 0, 0, 0 };
    region.extent = { m_width, m_height, 1 };

    m_impl->renderSystem->ReadTexture(*m_impl->colorTexture, region, dstImage);

    return result;
}

void RenderTexture::ReadPixelsAsync(ReadbackCallback callback) {
    if (!callback || !m_impl->colorTexture || !m_impl->renderSystem) {
        if (callback) callback(PixelData{});
        return;
    }

    // Create staging buffer for async readback
    uint32_t bytesPerPixel = GetBytesPerPixel(m_format);
    uint32_t dataSize = m_width * m_height * bytesPerPixel;

    LLGL::BufferDescriptor bufDesc;
    bufDesc.size = dataSize;
    bufDesc.bindFlags = 0;  // No binding, just for readback
    bufDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Read;
    bufDesc.miscFlags = LLGL::MiscFlags::NoInitialData;

    auto* stagingBuffer = m_impl->renderSystem->CreateBuffer(bufDesc);
    if (!stagingBuffer) {
        callback(PixelData{});
        return;
    }

    // Queue the readback request
    std::lock_guard<std::mutex> lock(m_impl->readbackMutex);
    RenderTextureImpl::ReadbackRequest req;
    req.callback = callback;
    req.stagingBuffer = stagingBuffer;
    req.width = m_width;
    req.height = m_height;
    req.format = m_format;
    req.copyIssued = false;
    m_impl->pendingReadbacks.push(req);
}

bool RenderTexture::IsReadbackPending() const {
    std::lock_guard<std::mutex> lock(m_impl->readbackMutex);
    return !m_impl->pendingReadbacks.empty();
}

void RenderTexture::CancelReadback() {
    std::lock_guard<std::mutex> lock(m_impl->readbackMutex);
    while (!m_impl->pendingReadbacks.empty()) {
        auto& req = m_impl->pendingReadbacks.front();
        if (req.stagingBuffer && m_impl->renderSystem) {
            m_impl->renderSystem->Release(*req.stagingBuffer);
        }
        m_impl->pendingReadbacks.pop();
    }
}

// ============================================================================
// Utility Functions Implementation
// ============================================================================

uint32_t GetBytesPerPixel(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA8:
        case PixelFormat::BGRA8:
            return 4;
        case PixelFormat::RGB8:
            return 3;
        case PixelFormat::R8:
            return 1;
        case PixelFormat::RG8:
            return 2;
        case PixelFormat::RGBA16F:
            return 8;
        case PixelFormat::RGBA32F:
            return 16;
        default:
            return 4;
    }
}

} // namespace ToyFrameV
