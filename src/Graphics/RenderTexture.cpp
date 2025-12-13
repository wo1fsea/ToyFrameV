/**
 * @file RenderTexture.cpp
 * @brief RenderTexture implementation using backend abstraction
 */

#include "ToyFrameV/Graphics/RenderTexture.h"
#include "Backend/IGraphicsBackend.h"
#include "ToyFrameV/Graphics/Context.h"
#include "ToyFrameV/Platform/FileDownload.h"

#include <cstring>

namespace ToyFrameV {

// ============================================================================
// PixelData - BMP Export
// ============================================================================

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

  // Use platform-abstracted file save/queue
  return Platform::SaveOrQueueFile(filename, bmpData);
}

// Static methods delegating to Platform abstraction
void PixelData::DownloadAllAsZip(const std::string &zipFilename) {
  Platform::DownloadQueuedFiles(zipFilename);
}

void PixelData::ClearPending() { Platform::ClearQueuedFiles(); }

size_t PixelData::GetPendingCount() { return Platform::GetQueuedFileCount(); }

bool PixelData::UsesQueuedDownload() { return Platform::UsesQueuedDownloads(); }

// ============================================================================
// RenderTexture Implementation
// ============================================================================

RenderTexture::RenderTexture(Graphics *graphics) : m_graphics(graphics) {}

RenderTexture::~RenderTexture() {
  if (m_backendHandle && m_graphics) {
    auto *backend = m_graphics->GetBackend();
    if (backend) {
      backend->DestroyRenderTexture(m_backendHandle);
    }
  }
}

bool RenderTexture::Initialize(const RenderTextureDesc &desc) {
  if (!m_graphics)
    return false;

  auto *backend = m_graphics->GetBackend();
  if (!backend)
    return false;

  m_width = desc.width;
  m_height = desc.height;
  m_format = desc.format;

  BackendRenderTextureDesc backendDesc;
  backendDesc.width = desc.width;
  backendDesc.height = desc.height;
  backendDesc.format = desc.format;
  backendDesc.hasDepth = desc.hasDepth;

  m_backendHandle = backend->CreateRenderTexture(backendDesc);
  return m_backendHandle != nullptr;
}

uint32_t RenderTexture::GetWidth() const { return m_width; }
uint32_t RenderTexture::GetHeight() const { return m_height; }
PixelFormat RenderTexture::GetFormat() const { return m_format; }

void RenderTexture::Resize(uint32_t width, uint32_t height) {
    if (width == m_width && height == m_height) return;
    if (!m_graphics || !m_backendHandle)
      return;

    auto *backend = m_graphics->GetBackend();
    if (!backend)
      return;

    // Cancel pending readbacks first
    CancelReadback();

    // Resize via backend
    if (backend->ResizeRenderTexture(m_backendHandle, width, height)) {
      m_width = width;
      m_height = height;
    }
}

PixelData RenderTexture::ReadPixels() {
    PixelData result;
    if (!m_graphics || !m_backendHandle)
      return result;

    auto *backend = m_graphics->GetBackend();
    if (!backend)
      return result;

    BackendPixelData backendData =
        backend->ReadRenderTexturePixels(m_backendHandle);

    result.data = std::move(backendData.data);
    result.width = backendData.width;
    result.height = backendData.height;
    result.format = backendData.format;

    return result;
}

void RenderTexture::ReadPixelsAsync(ReadbackCallback callback) {
  if (!callback || !m_graphics || !m_backendHandle) {
    if (callback)
      callback(PixelData{});
    return;
  }

    auto *backend = m_graphics->GetBackend();
    if (!backend) {
      callback(PixelData{});
      return;
    }

    // Wrap the callback to convert BackendPixelData to PixelData
    backend->ReadRenderTexturePixelsAsync(
        m_backendHandle, [callback](BackendPixelData backendData) {
          PixelData result;
          result.data = std::move(backendData.data);
          result.width = backendData.width;
          result.height = backendData.height;
          result.format = backendData.format;
          callback(std::move(result));
        });
}

bool RenderTexture::IsReadbackPending() const {
  if (!m_graphics || !m_backendHandle)
    return false;

  auto *backend = m_graphics->GetBackend();
  if (!backend)
    return false;

  return backend->IsReadbackPending(m_backendHandle);
}

void RenderTexture::CancelReadback() {
  if (!m_graphics || !m_backendHandle)
    return;

  auto *backend = m_graphics->GetBackend();
  if (!backend)
    return;

  backend->CancelReadback(m_backendHandle);
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
