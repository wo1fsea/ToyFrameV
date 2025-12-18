#pragma once

/**
 * @file AssetSystem.h
 * @brief Asset management system for ToyFrameV framework
 *
 * Provides unified asset loading and caching:
 * - Texture loading (PNG, JPG, BMP, etc.)
 * - Async loading support via Future<T>
 * - Asset caching to avoid duplicate loads
 * - Integration with IOSystem for path resolution
 */

#include "ToyFrameV/System.h"
#include "ToyFrameV/Graphics/Texture.h"
#include "ToyFrameV/Core/Threading.h"
#include "ToyFrameV/IOSystem.h"
#include "ToyFrameV/GraphicsSystem.h"
#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <unordered_map>

namespace ToyFrameV {

// Forward declarations
class App;
class Graphics;

/**
 * @brief Asset loading status
 */
enum class AssetStatus {
    NotLoaded,  // Asset not loaded yet
    Loading,    // Asset is being loaded asynchronously
    Loaded,     // Asset is loaded and ready
    Failed      // Asset failed to load
};

/**
 * @brief Callback for async asset loading
 */
template <typename T>
using AssetCallback = std::function<void(std::shared_ptr<T>)>;

/**
 * @brief Asset handle for tracking loaded assets
 */
template <typename T>
struct AssetHandle {
    std::shared_ptr<T> asset;
    AssetStatus status = AssetStatus::NotLoaded;
    std::string path;
    std::string errorMessage;

    bool IsReady() const { return status == AssetStatus::Loaded && asset != nullptr; }
    bool IsFailed() const { return status == AssetStatus::Failed; }
    bool IsLoading() const { return status == AssetStatus::Loading; }

    T* Get() const { return asset.get(); }
    T* operator->() const { return asset.get(); }
    explicit operator bool() const { return IsReady(); }
};

/**
 * @brief Asset System for loading and managing game assets
 *
 * The AssetSystem provides a centralized way to load and cache assets.
 * It integrates with IOSystem for path resolution and supports async loading.
 *
 * Usage:
 * @code
 * // Get AssetSystem from App
 * auto* assets = app->GetSystem<AssetSystem>();
 *
 * // Load texture synchronously
 * auto texture = assets->LoadTexture("assets://textures/logo.png");
 *
 * // Load texture asynchronously
 * assets->LoadTextureAsync("assets://textures/logo.png",
 *     [](std::shared_ptr<Texture> tex) {
 *         if (tex) {
 *             // Texture loaded successfully
 *         }
 *     });
 *
 * // Get cached texture
 * auto cached = assets->GetTexture("assets://textures/logo.png");
 * @endcode
 */
class AssetSystem : public System {
  public:
    AssetSystem() = default;
    ~AssetSystem() override = default;

    // System interface
    const char* GetName() const override { return "AssetSystem"; }
    int GetPriority() const override {
        // After IOSystem (10), before user systems (200)
        return 20;
    }

    std::vector<const std::type_info*> GetDependencies() const override {
        return {&typeid(IOSystem), &typeid(GraphicsSystem)};
    }

    bool Initialize(App* app) override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    // ==================== Texture Loading ====================

    /**
     * @brief Load a texture synchronously
     * @param path Asset path (supports assets://, file://, etc.)
     * @param generateMipmaps Whether to generate mipmaps
     * @return Shared pointer to texture, or nullptr on failure
     */
    std::shared_ptr<Texture> LoadTexture(const std::string& path, bool generateMipmaps = false);

    /**
     * @brief Load a texture asynchronously
     * @param path Asset path
     * @param callback Called when loading completes (may be nullptr on failure)
     * @param generateMipmaps Whether to generate mipmaps
     */
    void LoadTextureAsync(const std::string& path, AssetCallback<Texture> callback,
                          bool generateMipmaps = false);

    /**
     * @brief Get a cached texture
     * @param path Asset path
     * @return Shared pointer to texture, or nullptr if not cached
     */
    std::shared_ptr<Texture> GetTexture(const std::string& path) const;

    /**
     * @brief Check if a texture is loaded and cached
     * @param path Asset path
     * @return true if texture is in cache
     */
    bool IsTextureLoaded(const std::string& path) const;

    // ==================== Cache Management ====================

    /**
     * @brief Unload a specific texture from cache
     * @param path Asset path
     */
    void UnloadTexture(const std::string& path);

    /**
     * @brief Clear all cached textures
     */
    void ClearTextureCache();

    /**
     * @brief Get number of cached textures
     */
    size_t GetTextureCacheSize() const { return m_textureCache.size(); }

    // ==================== Direct Image Loading ====================

    /**
     * @brief Load image data from file (does not create GPU texture)
     * @param path Asset path
     * @return ImageData with pixels, empty if failed
     */
    ImageData LoadImageData(const std::string& path);

    /**
     * @brief Load image data asynchronously
     * @param path Asset path
     * @param callback Called with loaded image data
     */
    void LoadImageDataAsync(const std::string& path,
                            std::function<void(ImageData)> callback);

  private:
    // Process pending async callbacks
    void ProcessPendingCallbacks();

    // Internal texture creation from image data
    std::shared_ptr<Texture> CreateTextureFromImageData(const ImageData& image,
                                                         bool generateMipmaps);

    App* m_app = nullptr;
    Graphics* m_graphics = nullptr;
    IOSystem* m_ioSystem = nullptr;

    // Texture cache
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textureCache;

    // Pending async callbacks (to be executed on main thread)
    struct PendingTextureCallback {
        AssetCallback<Texture> callback;
        ImageData imageData;
        std::string path;
        bool generateMipmaps;
    };
    std::vector<PendingTextureCallback> m_pendingTextureCallbacks;
    Core::Mutex m_callbackMutex;
};

}  // namespace ToyFrameV
