/**
 * @file Texture.cpp
 * @brief Texture and image loading implementation
 */

#include "ToyFrameV/Graphics/Texture.h"
#include "ToyFrameV/Core/Log.h"
#include <cstdio>
#include <cstring>

// stb_image implementation
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace ToyFrameV {

// ============================================================================
// Texture Implementation
// ============================================================================

Texture::~Texture() {
    // Backend resources are cleaned up by the backend
    // They will be released when the Graphics context is destroyed
}

// ============================================================================
// Sampler Implementation
// ============================================================================

Sampler::~Sampler() {
    // Backend resources are cleaned up by the backend
}

// ============================================================================
// ImageData Implementation
// ============================================================================

ImageData ImageData::LoadFromFile(const std::string& path) {
    ImageData result;

    // Load file into memory using standard file I/O
    FILE* file = nullptr;
#ifdef _WIN32
    fopen_s(&file, path.c_str(), "rb");
#else
    file = fopen(path.c_str(), "rb");
#endif

    if (!file) {
        TOYFRAMEV_LOG_ERROR("Failed to open image file: {}", path);
        return result;
    }

    // Get file size using fseeko/ftello for large file support
#if defined(_WIN32)
    _fseeki64(file, 0, SEEK_END);
    int64_t fileSize64 = _ftelli64(file);
    _fseeki64(file, 0, SEEK_SET);
#else
    fseeko(file, 0, SEEK_END);
    off_t fileSize64 = ftello(file);
    fseeko(file, 0, SEEK_SET);
#endif

    if (fileSize64 <= 0 || fileSize64 > static_cast<int64_t>(SIZE_MAX)) {
        fclose(file);
        TOYFRAMEV_LOG_ERROR("Invalid image file size: {}", path);
        return result;
    }

    size_t fileSize = static_cast<size_t>(fileSize64);

    // Read file data
    std::vector<uint8_t> fileData(fileSize);
    size_t bytesRead = fread(fileData.data(), 1, fileSize, file);
    fclose(file);

    if (bytesRead != fileSize) {
        TOYFRAMEV_LOG_ERROR("Failed to read image file: {}", path);
        return result;
    }

    // Load from memory
    return LoadFromMemory(fileData.data(), fileData.size());
}

ImageData ImageData::LoadFromMemory(const void* data, size_t size) {
    ImageData result;

    if (!data || size == 0) {
        TOYFRAMEV_LOG_ERROR("Invalid image data");
        return result;
    }

    int width, height, channels;
    // Force 4 channels (RGBA) for consistency
    unsigned char* pixels = stbi_load_from_memory(
        static_cast<const unsigned char*>(data), static_cast<int>(size),
        &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        TOYFRAMEV_LOG_ERROR("Failed to decode image: {}", stbi_failure_reason());
        return result;
    }

    result.width = static_cast<uint32_t>(width);
    result.height = static_cast<uint32_t>(height);
    result.channels = 4;  // We always load as RGBA

    // Copy pixel data
    size_t dataSize = static_cast<size_t>(width) * static_cast<size_t>(height) * 4;
    result.pixels.resize(dataSize);
    memcpy(result.pixels.data(), pixels, dataSize);

    // Free stb_image allocated memory
    stbi_image_free(pixels);

    TOYFRAMEV_LOG_DEBUG("Loaded image: {}x{} ({} channels)", width, height, channels);

    return result;
}

}  // namespace ToyFrameV
