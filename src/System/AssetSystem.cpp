/**
 * @file AssetSystem.cpp
 * @brief Asset management system implementation
 */

#include "ToyFrameV/AssetSystem.h"
#include "ToyFrameV/App.h"
#include "ToyFrameV/Core/Log.h"
#include "ToyFrameV/Graphics.h"
#include "ToyFrameV/GraphicsSystem.h"
#include "ToyFrameV/IOSystem.h"

namespace ToyFrameV {

bool AssetSystem::Initialize(App* app) {
    System::Initialize(app);
    m_app = app;

    // Get required systems
    auto* graphicsSystem = app->GetSystemManager().GetSystem<GraphicsSystem>();
    if (!graphicsSystem) {
        TOYFRAMEV_LOG_ERROR("AssetSystem: GraphicsSystem not found");
        return false;
    }
    m_graphics = graphicsSystem->GetGraphics();

    m_ioSystem = app->GetSystemManager().GetSystem<IOSystem>();
    if (!m_ioSystem) {
        TOYFRAMEV_LOG_ERROR("AssetSystem: IOSystem not found");
        return false;
    }

    TOYFRAMEV_LOG_INFO("AssetSystem initialized");
    return true;
}

void AssetSystem::Update(float deltaTime) {
    ProcessPendingCallbacks();
}

void AssetSystem::Shutdown() {
    ClearTextureCache();
    m_pendingTextureCallbacks.clear();
    TOYFRAMEV_LOG_INFO("AssetSystem shutdown");
}

void AssetSystem::ProcessPendingCallbacks() {
    std::vector<PendingTextureCallback> callbacks;
    {
        Core::LockGuard<Core::Mutex> lock(m_callbackMutex);
        callbacks = std::move(m_pendingTextureCallbacks);
        m_pendingTextureCallbacks.clear();
    }

    for (auto& pending : callbacks) {
        std::shared_ptr<Texture> texture = nullptr;
        
        if (pending.imageData.IsValid()) {
            texture = CreateTextureFromImageData(pending.imageData, pending.generateMipmaps);
            
            if (texture) {
                // Cache the texture
                m_textureCache[pending.path] = texture;
            }
        }

        if (pending.callback) {
            pending.callback(texture);
        }
    }
}

// ==================== Texture Loading ====================

std::shared_ptr<Texture> AssetSystem::LoadTexture(const std::string& path, bool generateMipmaps) {
    // Check cache first
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }

    // Load image data
    ImageData imageData = LoadImageData(path);
    if (!imageData.IsValid()) {
        TOYFRAMEV_LOG_ERROR("Failed to load texture: {}", path);
        return nullptr;
    }

    // Create texture
    auto texture = CreateTextureFromImageData(imageData, generateMipmaps);
    if (!texture) {
        return nullptr;
    }

    // Cache and return
    m_textureCache[path] = texture;
    TOYFRAMEV_LOG_INFO("Loaded texture: {} ({}x{})", path, imageData.width, imageData.height);
    return texture;
}

void AssetSystem::LoadTextureAsync(const std::string& path, AssetCallback<Texture> callback,
                                    bool generateMipmaps) {
    // Check cache first
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        if (callback) {
            callback(it->second);
        }
        return;
    }

    // Load image data asynchronously on thread pool
    LoadImageDataAsync(path, [this, path, callback, generateMipmaps](ImageData imageData) {
        // Queue callback to be executed on main thread
        PendingTextureCallback pending;
        pending.callback = callback;
        pending.imageData = std::move(imageData);
        pending.path = path;
        pending.generateMipmaps = generateMipmaps;

        Core::LockGuard<Core::Mutex> lock(m_callbackMutex);
        m_pendingTextureCallbacks.push_back(std::move(pending));
    });
}

std::shared_ptr<Texture> AssetSystem::GetTexture(const std::string& path) const {
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        return it->second;
    }
    return nullptr;
}

bool AssetSystem::IsTextureLoaded(const std::string& path) const {
    return m_textureCache.find(path) != m_textureCache.end();
}

// ==================== Cache Management ====================

void AssetSystem::UnloadTexture(const std::string& path) {
    auto it = m_textureCache.find(path);
    if (it != m_textureCache.end()) {
        m_textureCache.erase(it);
        TOYFRAMEV_LOG_DEBUG("Unloaded texture: {}", path);
    }
}

void AssetSystem::ClearTextureCache() {
    size_t count = m_textureCache.size();
    m_textureCache.clear();
    TOYFRAMEV_LOG_DEBUG("Cleared texture cache ({} textures)", count);
}

// ==================== Image Loading ====================

ImageData AssetSystem::LoadImageData(const std::string& path) {
    // Use IOSystem to read the file
    IOResult result = m_ioSystem->ReadFile(path);
    
    if (!result.IsSuccess()) {
        TOYFRAMEV_LOG_ERROR("Failed to read image file: {} - {}", path, result.errorMessage);
        return ImageData{};
    }

    // Decode image from memory
    return ImageData::LoadFromMemory(result.Data(), result.Size());
}

void AssetSystem::LoadImageDataAsync(const std::string& path,
                                      std::function<void(ImageData)> callback) {
    // Use IOSystem async read
    m_ioSystem->ReadFileAsync(path, [callback](IOResult result) {
        if (!result.IsSuccess()) {
            if (callback) {
                callback(ImageData{});
            }
            return;
        }

        // Decode image from memory
        ImageData imageData = ImageData::LoadFromMemory(result.Data(), result.Size());
        if (callback) {
            callback(std::move(imageData));
        }
    });
}

// ==================== Internal Helpers ====================

std::shared_ptr<Texture> AssetSystem::CreateTextureFromImageData(const ImageData& image,
                                                                   bool generateMipmaps) {
    if (!m_graphics) {
        TOYFRAMEV_LOG_ERROR("AssetSystem: Graphics context not available");
        return nullptr;
    }

    auto texture = m_graphics->CreateTextureFromImage(image, generateMipmaps);
    if (texture) {
        return std::shared_ptr<Texture>(texture.release());
    }
    return nullptr;
}

}  // namespace ToyFrameV
