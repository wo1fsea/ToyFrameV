/**
 * @file RenderTexture.cpp
 * @brief RenderTexture implementation using LLGL
 */

#include "ToyFrameV/Graphics/RenderTexture.h"
#include "ToyFrameV/Graphics/Context.h"

#include <LLGL/LLGL.h>
#include <cstring>
#include <fstream>
#include <map>
#include <mutex>
#include <queue>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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

#ifdef __EMSCRIPTEN__
// Store pending images for batch ZIP download
static std::map<std::string, std::vector<uint8_t>> s_pendingImages;
static std::mutex s_pendingMutex;
#endif

std::vector<uint8_t> PixelData::ToBMP() const {
  std::vector<uint8_t> result;
  if (!IsValid())
    return result;
  if (format != PixelFormat::RGBA8 && format != PixelFormat::BGRA8) {
    return result; // Only support 32-bit formats for now
  }

  // BMP Header (14 bytes)
  uint32_t rowSize =
      ((width * 4 + 3) / 4) * 4; // Rows padded to 4-byte boundary
  uint32_t pixelDataSize = rowSize * height;
  uint32_t fileSize = 54 + pixelDataSize;

  result.resize(fileSize);

  // BMP Header
  result[0] = 'B';
  result[1] = 'M';
  std::memcpy(&result[2], &fileSize, 4);
  // Reserved (bytes 6-9) already 0
  uint32_t dataOffset = 54;
  std::memcpy(&result[10], &dataOffset, 4);

  // DIB Header (40 bytes) - BITMAPINFOHEADER
  uint32_t dibSize = 40;
  std::memcpy(&result[14], &dibSize, 4);
  std::memcpy(&result[18], &width, 4);
  int32_t negHeight = -static_cast<int32_t>(height); // Negative for top-down
  std::memcpy(&result[22], &negHeight, 4);
  uint16_t colorPlanes = 1;
  std::memcpy(&result[26], &colorPlanes, 2);
  uint16_t bitsPerPixel = 32;
  std::memcpy(&result[28], &bitsPerPixel, 2);
  // Compression, imageSize, and other fields left at 0

  // Write pixel data (BMP uses BGRA order)
  size_t writeOffset = 54;
  if (format == PixelFormat::RGBA8) {
    // Convert RGBA to BGRA
    for (uint32_t y = 0; y < height; ++y) {
      for (uint32_t x = 0; x < width; ++x) {
        size_t srcIdx = (y * width + x) * 4;
        result[writeOffset++] = data[srcIdx + 2]; // B
        result[writeOffset++] = data[srcIdx + 1]; // G
        result[writeOffset++] = data[srcIdx + 0]; // R
        result[writeOffset++] = data[srcIdx + 3]; // A
      }
      // Padding to 4-byte boundary
      uint32_t padding = rowSize - width * 4;
      for (uint32_t p = 0; p < padding; ++p) {
        result[writeOffset++] = 0;
      }
    }
  } else {
    // Already BGRA
    for (uint32_t y = 0; y < height; ++y) {
      std::memcpy(&result[writeOffset], &data[y * width * 4], width * 4);
      writeOffset += rowSize;
    }
  }

  return result;
}

bool PixelData::SaveToBMP(const std::string &filename) const {
  auto bmpData = ToBMP();
  if (bmpData.empty())
    return false;

#ifdef __EMSCRIPTEN__
  // Queue for later ZIP download
  std::lock_guard<std::mutex> lock(s_pendingMutex);
  s_pendingImages[filename] = std::move(bmpData);
  return true;
#else
  // Write directly to file on Desktop
  std::ofstream file(filename, std::ios::binary);
  if (!file)
    return false;
  file.write(reinterpret_cast<const char *>(bmpData.data()), bmpData.size());
  return file.good();
#endif
}

#ifdef __EMSCRIPTEN__
void PixelData::DownloadAllAsZip(const std::string &zipFilename) {
  std::lock_guard<std::mutex> lock(s_pendingMutex);

  if (s_pendingImages.empty())
    return;

  // Build buffer with all images data
  // Format: [nameLen:u32, name:char[], padding, dataLen:u32, data:u8[],
  // padding] * N
  size_t bufferSize = 0;
  for (const auto &[name, data] : s_pendingImages) {
    bufferSize += 4 + name.size();      // nameLen + name
    bufferSize = (bufferSize + 3) & ~3; // align
    bufferSize += 4 + data.size();      // dataLen + data
    bufferSize = (bufferSize + 3) & ~3; // align
  }

  std::vector<uint8_t> buffer(bufferSize);
  size_t offset = 0;

  for (const auto &[name, data] : s_pendingImages) {
    // Write name length
    uint32_t nameLen = static_cast<uint32_t>(name.size());
    std::memcpy(&buffer[offset], &nameLen, 4);
    offset += 4;

    // Write name
    std::memcpy(&buffer[offset], name.data(), name.size());
    offset += name.size();
    offset = (offset + 3) & ~3;

    // Write data length
    uint32_t dataLen = static_cast<uint32_t>(data.size());
    std::memcpy(&buffer[offset], &dataLen, 4);
    offset += 4;

    // Write data
    std::memcpy(&buffer[offset], data.data(), data.size());
    offset += data.size();
    offset = (offset + 3) & ~3;
  }

  int fileCount = static_cast<int>(s_pendingImages.size());

  // Call JavaScript to create ZIP and download
  EM_ASM(
      {
        // CRC32 calculation function
        var crc32Table = null;
        function makeCRC32Table() {
          var c;
          var table = [];
          for (var n = 0; n < 256; n++) {
            c = n;
            for (var k = 0; k < 8; k++) {
              c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
            }
            table[n] = c;
          }
          return table;
        }
        function crc32(data) {
          if (!crc32Table) crc32Table = makeCRC32Table();
          var crc = 0 ^ (-1);
          for (var i = 0; i < data.length; i++) {
            crc = (crc >>> 8) ^ crc32Table[(crc ^ data[i]) & 0xFF];
          }
          return (crc ^ (-1)) >>> 0;
        }

        var files = [];
        var ptr = $0;
        var count = $1;
        var offset = 0;

        for (var i = 0; i < count; i++) {
          var nameLen = HEAPU32[(ptr + offset) >> 2];
          offset += 4;

          var name = "";
          for (var j = 0; j < nameLen; j++) {
            name += String.fromCharCode(HEAPU8[ptr + offset + j]);
          }
          offset += nameLen;
          offset = (offset + 3) & ~3;

          var dataLen = HEAPU32[(ptr + offset) >> 2];
          offset += 4;

          var data = new Uint8Array(dataLen);
          for (var j = 0; j < dataLen; j++) {
            data[j] = HEAPU8[ptr + offset + j];
          }
          offset += dataLen;
          offset = (offset + 3) & ~3;

          files.push({name : name, data : data, crc: crc32(data)});
        }

        // Create ZIP
        var centralDir = [];
        var localHeaders = [];
        var localOffset = 0;

        for (var i = 0; i < files.length; i++) {
          var file = files[i];
          var nameBytes = new TextEncoder().encode(file.name);

          var localHeader = new Uint8Array(30 + nameBytes.length);
          var view = new DataView(localHeader.buffer);
          view.setUint32(0, 0x04034b50, true);   // Local file header signature
          view.setUint16(4, 20, true);           // Version needed to extract
          view.setUint16(6, 0, true);            // General purpose bit flag
          view.setUint16(8, 0, true);            // Compression method (0 = stored)
          view.setUint16(10, 0, true);           // File last modification time
          view.setUint16(12, 0, true);           // File last modification date
          view.setUint32(14, file.crc, true);    // CRC-32
          view.setUint32(18, file.data.length, true);  // Compressed size
          view.setUint32(22, file.data.length, true);  // Uncompressed size
          view.setUint16(26, nameBytes.length, true);  // File name length
          view.setUint16(28, 0, true);           // Extra field length
          localHeader.set(nameBytes, 30);
          localHeaders.push(localHeader);

          var cdEntry = new Uint8Array(46 + nameBytes.length);
          var cdView = new DataView(cdEntry.buffer);
          cdView.setUint32(0, 0x02014b50, true);  // Central directory signature
          cdView.setUint16(4, 20, true);          // Version made by
          cdView.setUint16(6, 20, true);          // Version needed to extract
          cdView.setUint16(8, 0, true);           // General purpose bit flag
          cdView.setUint16(10, 0, true);          // Compression method
          cdView.setUint16(12, 0, true);          // File last modification time
          cdView.setUint16(14, 0, true);          // File last modification date
          cdView.setUint32(16, file.crc, true);   // CRC-32
          cdView.setUint32(20, file.data.length, true);  // Compressed size
          cdView.setUint32(24, file.data.length, true);  // Uncompressed size
          cdView.setUint16(28, nameBytes.length, true);  // File name length
          cdView.setUint16(30, 0, true);          // Extra field length
          cdView.setUint16(32, 0, true);          // File comment length
          cdView.setUint16(34, 0, true);          // Disk number start
          cdView.setUint16(36, 0, true);          // Internal file attributes
          cdView.setUint32(38, 0, true);          // External file attributes
          cdView.setUint32(42, localOffset, true); // Relative offset of local header
          cdEntry.set(nameBytes, 46);
          centralDir.push(cdEntry);

          localOffset += localHeader.length + file.data.length;
        }

        var totalSize = localOffset;
        var cdOffset = totalSize;
        var cdSize = 0;
        for (var i = 0; i < centralDir.length; i++) {
          cdSize += centralDir[i].length;
        }
        totalSize += cdSize + 22;

        var zip = new Uint8Array(totalSize);
        var zipOffset = 0;

        for (var i = 0; i < files.length; i++) {
          zip.set(localHeaders[i], zipOffset);
          zipOffset += localHeaders[i].length;
          zip.set(files[i].data, zipOffset);
          zipOffset += files[i].data.length;
        }

        for (var i = 0; i < centralDir.length; i++) {
          zip.set(centralDir[i], zipOffset);
          zipOffset += centralDir[i].length;
        }

        var eocd = new Uint8Array(22);
        var eocdView = new DataView(eocd.buffer);
        eocdView.setUint32(0, 0x06054b50, true);  // End of central directory signature
        eocdView.setUint16(4, 0, true);           // Number of this disk
        eocdView.setUint16(6, 0, true);           // Disk where central directory starts
        eocdView.setUint16(8, files.length, true);  // Number of central directory records on this disk
        eocdView.setUint16(10, files.length, true); // Total number of central directory records
        eocdView.setUint32(12, cdSize, true);     // Size of central directory
        eocdView.setUint32(16, cdOffset, true);   // Offset of start of central directory
        eocdView.setUint16(20, 0, true);          // Comment length
        zip.set(eocd, zipOffset);

        console.log('ZIP created with ' + files.length + ' files, size: ' +
                    zip.length + ' bytes');

        // Use the global download function defined in template.html
        // This uses requestAnimationFrame to properly escape WASM context
        var filename = UTF8ToString($2);
        if (window.ToyFrameV_DownloadBlob) {
            window.ToyFrameV_DownloadBlob(zip, filename);
        } else {
            console.error('ToyFrameV_DownloadBlob not found! Fallback to direct download.');
            var blob = new Blob([zip], { type: 'application/octet-stream' });
            var url = URL.createObjectURL(blob);
            var a = document.createElement('a');
            a.href = url;
            a.download = filename;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        }
      },
      buffer.data(), fileCount, zipFilename.c_str());

  s_pendingImages.clear();
}

void PixelData::ClearPending() {
  std::lock_guard<std::mutex> lock(s_pendingMutex);
  s_pendingImages.clear();
}

size_t PixelData::GetPendingCount() {
  std::lock_guard<std::mutex> lock(s_pendingMutex);
  return s_pendingImages.size();
}
#endif

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
