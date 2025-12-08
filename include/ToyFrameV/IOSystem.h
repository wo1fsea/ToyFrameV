#pragma once

/**
 * @file IOSystem.h
 * @brief I/O subsystem for ToyFrameV framework
 *
 * Provides unified file I/O abstraction supporting:
 * - Local file system access
 * - Network/HTTP resources (async)
 * - Platform-specific paths (assets, documents, etc.)
 * - Zero-copy buffer management
 */

#include "ToyFrameV/System.h"
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ToyFrameV {

// Forward declarations
class App;
class IORequest;

/**
 * @brief I/O operation result status
 */
enum class IOStatus {
    Success,       // Operation completed successfully
    Pending,       // Operation is in progress (async)
    NotFound,      // File/resource not found
    AccessDenied,  // Permission denied
    NetworkError,  // Network connection failed
    Timeout,       // Operation timed out
    InvalidPath,   // Invalid path format
    IOError,       // General I/O error
    Cancelled      // Operation was cancelled
};

/**
 * @brief Data buffer with move semantics (zero-copy)
 *
 * Avoids unnecessary copies by using unique_ptr internally.
 * Provides convenient accessors for different data formats.
 */
class IOBuffer {
  public:
    IOBuffer() = default;

    explicit IOBuffer(size_t size)
        : m_data(size > 0 ? new uint8_t[size] : nullptr), m_size(size) {}

    IOBuffer(const void *data, size_t size) : IOBuffer(size) {
        if (data && size > 0) {
            std::memcpy(m_data.get(), data, size);
        }
    }

    // Move semantics
    IOBuffer(IOBuffer &&other) noexcept
        : m_data(std::move(other.m_data)), m_size(other.m_size) {
        other.m_size = 0;
    }

    IOBuffer &operator=(IOBuffer &&other) noexcept {
        if (this != &other) {
            m_data = std::move(other.m_data);
            m_size = other.m_size;
            other.m_size = 0;
        }
        return *this;
    }

    // Disable copy (force move semantics for efficiency)
    IOBuffer(const IOBuffer &) = delete;
    IOBuffer &operator=(const IOBuffer &) = delete;

    // Data accessors
    uint8_t *Data() { return m_data.get(); }
    const uint8_t *Data() const { return m_data.get(); }
    size_t Size() const { return m_size; }
    bool Empty() const { return m_size == 0 || m_data == nullptr; }

    // String accessors (assumes null-terminated or uses size)
    const char *AsString() const {
        return m_data ? reinterpret_cast<const char *>(m_data.get()) : "";
    }

    std::string_view AsStringView() const {
        return m_data ? std::string_view(reinterpret_cast<const char *>(m_data.get()), m_size)
                      : std::string_view();
    }

    std::string ToString() const {
        return m_data ? std::string(reinterpret_cast<const char *>(m_data.get()), m_size)
                      : std::string();
    }

    // Convert to vector (copies data - use when STL compatibility needed)
    std::vector<uint8_t> ToVector() const {
        if (!m_data || m_size == 0)
            return {};
        return std::vector<uint8_t>(m_data.get(), m_data.get() + m_size);
    }

    // Release ownership (zero-copy transfer)
    std::unique_ptr<uint8_t[]> Release() {
        m_size = 0;
        return std::move(m_data);
    }

    // Clear buffer
    void Clear() {
        m_data.reset();
        m_size = 0;
    }

  private:
    std::unique_ptr<uint8_t[]> m_data;
    size_t m_size = 0;
};

/**
 * @brief I/O operation result
 */
struct IOResult {
    IOStatus status = IOStatus::Success;
    IOBuffer buffer;
    std::string errorMessage;

    bool IsSuccess() const { return status == IOStatus::Success; }
    bool IsPending() const { return status == IOStatus::Pending; }
    bool IsError() const { return status != IOStatus::Success && status != IOStatus::Pending; }

    // Convenience accessors delegating to buffer
    const uint8_t *Data() const { return buffer.Data(); }
    size_t Size() const { return buffer.Size(); }
    const char *AsString() const { return buffer.AsString(); }
    std::string_view AsStringView() const { return buffer.AsStringView(); }
    std::string ToString() const { return buffer.ToString(); }
};

/**
 * @brief Callback for async I/O operations
 */
using IOCallback = std::function<void(IOResult)>;

/**
 * @brief Progress callback for async operations
 * @param bytesLoaded Bytes loaded so far
 * @param totalBytes Total bytes (0 if unknown)
 */
using IOProgressCallback = std::function<void(size_t bytesLoaded, size_t totalBytes)>;

/**
 * @brief Resource location type
 */
enum class IOPathType {
    Auto,       // Auto-detect based on path prefix
    LocalFile,  // Local file system
    Assets,     // Application assets (read-only, bundled)
    Documents,  // User documents directory (read-write)
    Cache,      // Cache directory (may be cleared by OS)
    Temp,       // Temporary directory
    HTTP,       // HTTP URL
    HTTPS       // HTTPS URL
};

/**
 * @brief I/O request handle for async operations
 */
class IORequest {
  public:
    virtual ~IORequest() = default;

    /**
     * @brief Check if the request is complete
     */
    virtual bool IsComplete() const = 0;

    /**
     * @brief Get the result (only valid after IsComplete() returns true)
     */
    virtual const IOResult &GetResult() const = 0;

    /**
     * @brief Cancel the pending request
     */
    virtual void Cancel() = 0;

    /**
     * @brief Wait for the request to complete (blocking)
     * @param timeoutMs Timeout in milliseconds (0 = infinite)
     * @return true if completed, false if timed out
     */
    virtual bool Wait(uint32_t timeoutMs = 0) = 0;

    /**
     * @brief Set progress callback
     */
    virtual void SetProgressCallback(IOProgressCallback callback) = 0;
};

/**
 * @brief Shared pointer to IORequest
 */
using IORequestPtr = std::shared_ptr<IORequest>;

/**
 * @brief I/O subsystem
 *
 * Provides unified file I/O abstraction for the framework.
 * Supports local files, bundled assets, and network resources.
 *
 * Path formats:
 * - Local file: "path/to/file.txt" or "file://path/to/file.txt"
 * - Assets: "assets://textures/logo.png"
 * - Documents: "documents://saves/game.sav"
 * - Cache: "cache://downloaded/data.bin"
 * - HTTP: "http://example.com/resource.json"
 * - HTTPS: "https://example.com/resource.json"
 *
 * Thread safety:
 * - Read operations are thread-safe
 * - Async callbacks are called on the main thread during Update()
 */
class IOSystem : public System {
  public:
    IOSystem() = default;
    ~IOSystem() override = default;

    // System interface
    const char *GetName() const override { return "IOSystem"; }
    int GetPriority() const override {
        // Early priority - after WindowSystem, before InputSystem
        return static_cast<int>(SystemPriority::Platform) + 10;
    }

    bool Initialize(App *app) override;
    void Update(float deltaTime) override;
    void Shutdown() override;

    // ==================== Synchronous API ====================

    /**
     * @brief Read entire file synchronously
     * @param path File path or URL
     * @param pathType Path type hint (Auto = detect from path)
     * @return IOResult containing data or error
     */
    IOResult ReadFile(const std::string &path, IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Read file as string synchronously
     * @param path File path
     * @param pathType Path type hint
     * @return File contents as string, empty on error
     */
    std::string ReadTextFile(const std::string &path, IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Write data to file synchronously
     * @param path File path (local only, not URLs)
     * @param data Data to write
     * @param size Data size in bytes
     * @param pathType Path type hint
     * @return IOResult with status
     */
    IOResult WriteFile(const std::string &path, const void *data, size_t size,
                       IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Write buffer to file synchronously
     */
    IOResult WriteFile(const std::string &path, const IOBuffer &buffer,
                       IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Write string to file synchronously
     */
    IOResult WriteTextFile(const std::string &path, const std::string &content,
                           IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Check if file/resource exists
     * @param path File path or URL
     * @param pathType Path type hint
     * @return true if exists
     */
    bool Exists(const std::string &path, IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Delete a file
     * @param path File path (local only)
     * @return true if deleted successfully
     */
    bool Delete(const std::string &path, IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Get file size
     * @param path File path
     * @param pathType Path type hint
     * @return File size in bytes, 0 if not found
     */
    size_t GetFileSize(const std::string &path, IOPathType pathType = IOPathType::Auto);

    // ==================== Asynchronous API ====================

    /**
     * @brief Read file asynchronously
     * @param path File path or URL
     * @param callback Callback when complete (called on main thread)
     * @param pathType Path type hint
     * @return Request handle for tracking/cancellation
     */
    IORequestPtr ReadFileAsync(const std::string &path, IOCallback callback,
                               IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Write file asynchronously
     * @param path File path
     * @param buffer Data to write (moved)
     * @param callback Callback when complete
     * @param pathType Path type hint
     * @return Request handle
     */
    IORequestPtr WriteFileAsync(const std::string &path, IOBuffer buffer, IOCallback callback,
                                IOPathType pathType = IOPathType::Auto);

    // ==================== Path Utilities ====================

    /**
     * @brief Resolve a path to absolute file system path
     * @param path Virtual path
     * @param pathType Path type hint
     * @return Resolved absolute path
     */
    std::string ResolvePath(const std::string &path, IOPathType pathType = IOPathType::Auto);

    /**
     * @brief Get the assets directory
     */
    const std::string &GetAssetsPath() const { return m_assetsPath; }

    /**
     * @brief Get the documents directory
     */
    const std::string &GetDocumentsPath() const { return m_documentsPath; }

    /**
     * @brief Get the cache directory
     */
    const std::string &GetCachePath() const { return m_cachePath; }

    /**
     * @brief Get the temp directory
     */
    const std::string &GetTempPath() const { return m_tempPath; }

    /**
     * @brief Set custom assets path (for development)
     */
    void SetAssetsPath(const std::string &path) { m_assetsPath = path; }

    // ==================== Network Configuration ====================

    /**
     * @brief Set default timeout for network operations
     * @param timeoutMs Timeout in milliseconds
     */
    void SetNetworkTimeout(uint32_t timeoutMs) { m_networkTimeout = timeoutMs; }

    /**
     * @brief Get network timeout
     */
    uint32_t GetNetworkTimeout() const { return m_networkTimeout; }

    /**
     * @brief Check if network I/O is available on this platform
     */
    bool IsNetworkAvailable() const { return m_networkAvailable; }

  private:
    // Path detection and resolution
    IOPathType DetectPathType(const std::string &path) const;
    std::string StripScheme(const std::string &path) const;

    // Platform-specific initialization
    void InitializePaths();

    // Ensure directory exists for file path
    bool EnsureDirectoryExists(const std::string &filePath);

    // Process pending async callbacks on main thread
    void ProcessPendingCallbacks();

    // Platform directories
    std::string m_assetsPath;
    std::string m_documentsPath;
    std::string m_cachePath;
    std::string m_tempPath;

    // Network settings
    uint32_t m_networkTimeout = 30000;  // 30 seconds default
    bool m_networkAvailable = false;

    // Pending async operations
    struct PendingCallback {
        IOCallback callback;
        IORequestPtr request;  // Hold reference to request for result access
        
        PendingCallback(IOCallback cb, IORequestPtr req)
            : callback(std::move(cb)), request(std::move(req)) {}
    };
    std::vector<PendingCallback> m_pendingCallbacks;
    // TODO: Add mutex for thread safety when using real async
};

}  // namespace ToyFrameV
