#include "ToyFrameV/IOSystem.h"
#include "ToyFrameV/App.h"
#include "ToyFrameV/Core/Log.h"
#include "ToyFrameV/Platform.h"
#include "ToyFrameV/Platform/FileSystem.h"
#include <cstring>
#include <fstream>
#include <sstream>

namespace ToyFrameV {

// ============================================================================
// SimpleIORequest - Basic async request implementation
// ============================================================================

class SimpleIORequest : public IORequest {
  public:
    bool IsComplete() const override { return m_complete; }
    const IOResult &GetResult() const override { return m_result; }
    void Cancel() override { m_cancelled = true; }
    bool Wait(uint32_t timeoutMs) override {
        // For now, sync operations complete immediately
        return m_complete;
    }
    void SetProgressCallback(IOProgressCallback callback) override {
        m_progressCallback = std::move(callback);
    }

    void SetResult(IOResult result) {
        m_result = std::move(result);
        m_complete = true;
    }

    bool IsCancelled() const { return m_cancelled; }

  private:
    IOResult m_result;
    bool m_complete = false;
    bool m_cancelled = false;
    IOProgressCallback m_progressCallback;
};

// ============================================================================
// IOSystem Implementation
// ============================================================================

bool IOSystem::Initialize(App *app) {
    System::Initialize(app);
    InitializePaths();

    m_networkAvailable = Platform::IsNetworkAvailable();

    TOYFRAMEV_LOG_DEBUG("IOSystem initialized");
    TOYFRAMEV_LOG_DEBUG("  Assets: {}", m_assetsPath);
    TOYFRAMEV_LOG_DEBUG("  Documents: {}", m_documentsPath);
    TOYFRAMEV_LOG_DEBUG("  Cache: {}", m_cachePath);
    TOYFRAMEV_LOG_DEBUG("  Temp: {}", m_tempPath);

    return true;
}

void IOSystem::Update(float deltaTime) { ProcessPendingCallbacks(); }

void IOSystem::Shutdown() { m_pendingCallbacks.clear(); }

void IOSystem::InitializePaths() {
  m_assetsPath = Platform::GetAssetsPath();
  m_documentsPath = Platform::GetDocumentsPath();
  m_cachePath = Platform::GetCachePath();
  m_tempPath = Platform::GetTempDirectoryPath();
}

IOPathType IOSystem::DetectPathType(const std::string &path) const {
    if (path.empty())
        return IOPathType::LocalFile;

    // Check URL schemes
    if (path.compare(0, 7, "http://") == 0)
        return IOPathType::HTTP;
    if (path.compare(0, 8, "https://") == 0)
        return IOPathType::HTTPS;
    if (path.compare(0, 9, "assets://") == 0)
        return IOPathType::Assets;
    if (path.compare(0, 12, "documents://") == 0)
        return IOPathType::Documents;
    if (path.compare(0, 8, "cache://") == 0)
        return IOPathType::Cache;
    if (path.compare(0, 7, "temp://") == 0)
        return IOPathType::Temp;
    if (path.compare(0, 7, "file://") == 0)
        return IOPathType::LocalFile;

    return IOPathType::LocalFile;
}

std::string IOSystem::StripScheme(const std::string &path) const {
    size_t pos = path.find("://");
    if (pos != std::string::npos) {
        return path.substr(pos + 3);
    }
    return path;
}

std::string IOSystem::ResolvePath(const std::string &path, IOPathType pathType) {
    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    std::string cleanPath = Platform::NormalizePath(StripScheme(path));

    switch (pathType) {
        case IOPathType::Assets:
          return Platform::JoinPath(m_assetsPath, cleanPath);
        case IOPathType::Documents:
          return Platform::JoinPath(m_documentsPath, cleanPath);
        case IOPathType::Cache:
          return Platform::JoinPath(m_cachePath, cleanPath);
        case IOPathType::Temp:
          return Platform::JoinPath(m_tempPath, cleanPath);
        case IOPathType::HTTP:
        case IOPathType::HTTPS:
            return path;  // Return URL as-is
        case IOPathType::LocalFile:
        default:
            return cleanPath;
    }
}

bool IOSystem::EnsureDirectoryExists(const std::string &filePath) {
    // Find the directory part of the path
    size_t lastSlash = filePath.find_last_of("\\/");
    if (lastSlash == std::string::npos) {
        return true;  // No directory in path
    }

    std::string dirPath = filePath.substr(0, lastSlash);
    if (dirPath.empty()) {
        return true;
    }

    return Platform::EnsureDirectoryExists(dirPath);
}

IOResult IOSystem::ReadFile(const std::string &path, IOPathType pathType) {
    IOResult result;

    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    // Network paths not supported for sync read (use async)
    if (pathType == IOPathType::HTTP || pathType == IOPathType::HTTPS) {
        result.status = IOStatus::IOError;
        result.errorMessage = "Use ReadFileAsync for network resources";
        return result;
    }

    std::string resolvedPath = ResolvePath(path, pathType);

    std::ifstream file(resolvedPath, std::ios::binary | std::ios::ate);
    if (!file) {
        result.status = IOStatus::NotFound;
        result.errorMessage = "File not found: " + resolvedPath;
        return result;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size < 0) {
        result.status = IOStatus::IOError;
        result.errorMessage = "Failed to get file size: " + resolvedPath;
        return result;
    }

    result.buffer = IOBuffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char *>(result.buffer.Data()), size)) {
        result.status = IOStatus::IOError;
        result.errorMessage = "Failed to read file: " + resolvedPath;
        result.buffer.Clear();
        return result;
    }

    result.status = IOStatus::Success;
    return result;
}

std::string IOSystem::ReadTextFile(const std::string &path, IOPathType pathType) {
    IOResult result = ReadFile(path, pathType);
    if (!result.IsSuccess()) {
        return "";
    }
    return result.ToString();
}

IOResult IOSystem::WriteFile(const std::string &path, const void *data, size_t size,
                             IOPathType pathType) {
    IOResult result;

    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    // Can't write to network or read-only locations
    if (pathType == IOPathType::HTTP || pathType == IOPathType::HTTPS) {
        result.status = IOStatus::AccessDenied;
        result.errorMessage = "Cannot write to network URL";
        return result;
    }
    if (pathType == IOPathType::Assets) {
        result.status = IOStatus::AccessDenied;
        result.errorMessage = "Assets directory is read-only";
        return result;
    }

    std::string resolvedPath = ResolvePath(path, pathType);

    // Ensure directory exists
    if (!EnsureDirectoryExists(resolvedPath)) {
        result.status = IOStatus::AccessDenied;
        result.errorMessage = "Cannot create directory for: " + resolvedPath;
        return result;
    }

    std::ofstream file(resolvedPath, std::ios::binary);
    if (!file) {
        result.status = IOStatus::AccessDenied;
        result.errorMessage = "Cannot open file for writing: " + resolvedPath;
        return result;
    }

    if (data && size > 0) {
        if (!file.write(reinterpret_cast<const char *>(data), size)) {
            result.status = IOStatus::IOError;
            result.errorMessage = "Failed to write file: " + resolvedPath;
            return result;
        }
    }

    result.status = IOStatus::Success;
    return result;
}

IOResult IOSystem::WriteFile(const std::string &path, const IOBuffer &buffer, IOPathType pathType) {
    return WriteFile(path, buffer.Data(), buffer.Size(), pathType);
}

IOResult IOSystem::WriteTextFile(const std::string &path, const std::string &content,
                                 IOPathType pathType) {
    return WriteFile(path, content.data(), content.size(), pathType);
}

bool IOSystem::Exists(const std::string &path, IOPathType pathType) {
    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    // Can't check network resources synchronously
    if (pathType == IOPathType::HTTP || pathType == IOPathType::HTTPS) {
        return false;
    }

    std::string resolvedPath = ResolvePath(path, pathType);
    std::ifstream file(resolvedPath);
    return file.good();
}

bool IOSystem::Delete(const std::string &path, IOPathType pathType) {
    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    if (pathType == IOPathType::HTTP || pathType == IOPathType::HTTPS ||
        pathType == IOPathType::Assets) {
        return false;
    }

    std::string resolvedPath = ResolvePath(path, pathType);
    return std::remove(resolvedPath.c_str()) == 0;
}

size_t IOSystem::GetFileSize(const std::string &path, IOPathType pathType) {
    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    if (pathType == IOPathType::HTTP || pathType == IOPathType::HTTPS) {
        return 0;
    }

    std::string resolvedPath = ResolvePath(path, pathType);
    std::ifstream file(resolvedPath, std::ios::binary | std::ios::ate);
    if (!file) {
        return 0;
    }

    std::streamsize size = file.tellg();
    return size > 0 ? static_cast<size_t>(size) : 0;
}

IORequestPtr IOSystem::ReadFileAsync(const std::string &path, IOCallback callback,
                                     IOPathType pathType) {
    auto request = std::make_shared<SimpleIORequest>();

    if (pathType == IOPathType::Auto) {
        pathType = DetectPathType(path);
    }

    // TODO: For HTTP/HTTPS, use platform-specific async fetch
    // For now, just do sync read and queue callback
    IOResult result = ReadFile(path, pathType);
    request->SetResult(std::move(result));

    // Queue callback to be called on main thread in Update()
    if (callback) {
        m_pendingCallbacks.emplace_back(std::move(callback), request);
    }

    return request;
}

IORequestPtr IOSystem::WriteFileAsync(const std::string &path, IOBuffer buffer, IOCallback callback,
                                      IOPathType pathType) {
    auto request = std::make_shared<SimpleIORequest>();

    // Perform sync write and queue callback
    IOResult result = WriteFile(path, buffer, pathType);
    request->SetResult(std::move(result));

    if (callback) {
        m_pendingCallbacks.emplace_back(std::move(callback), request);
    }

    return request;
}

void IOSystem::ProcessPendingCallbacks() {
    if (m_pendingCallbacks.empty()) {
        return;
    }

    // Move callbacks to local vector to allow new callbacks during processing
    auto callbacks = std::move(m_pendingCallbacks);
    m_pendingCallbacks.clear();

    for (auto &pending : callbacks) {
        if (pending.callback && pending.request) {
            // Create a copy of the result for the callback
            // The request holds the original result
            const auto &result = pending.request->GetResult();
            // We need to create a new IOResult since the original is const
            IOResult callbackResult;
            callbackResult.status = result.status;
            callbackResult.errorMessage = result.errorMessage;
            // For buffer, we create a copy if needed
            if (result.Data() && result.Size() > 0) {
                callbackResult.buffer = IOBuffer(result.Data(), result.Size());
            }
            pending.callback(std::move(callbackResult));
        }
    }
}

}  // namespace ToyFrameV
