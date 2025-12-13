/**
 * @file FileSystemWeb.cpp
 * @brief Web/Emscripten implementation of file system utilities
 */

#include "ToyFrameV/Platform/FileSystem.h"

namespace ToyFrameV {
namespace Platform {

std::string GetAssetsPath() {
    return "/assets";
}

std::string GetDocumentsPath() {
    return "/home/web_user";
}

std::string GetCachePath() {
    return "/tmp/cache";
}

std::string GetTempDirectoryPath() {
    return "/tmp";
}

bool EnsureDirectoryExists(const std::string& /*dirPath*/) {
    // On Web, we use the virtual file system which doesn't require
    // directory creation in the same way
    return true;
}

std::string NormalizePath(const std::string& path) {
    std::string result = path;
    for (char& c : result) {
        if (c == '\\') {
            c = '/';
        }
    }
    return result;
}

std::string JoinPath(const std::string& base, const std::string& relative) {
    if (base.empty()) {
        return NormalizePath(relative);
    }
    if (relative.empty()) {
        return NormalizePath(base);
    }
    
    std::string result = base;
    // Remove trailing separator from base
    if (!result.empty() && (result.back() == '/' || result.back() == '\\')) {
        result.pop_back();
    }
    // Remove leading separator from relative
    std::string rel = relative;
    if (!rel.empty() && (rel.front() == '/' || rel.front() == '\\')) {
        rel = rel.substr(1);
    }
    
    return NormalizePath(result + "/" + rel);
}

bool IsNetworkAvailable() {
    // Web always has network access
    return true;
}

} // namespace Platform
} // namespace ToyFrameV
