/**
 * @file FileSystemUnix.cpp
 * @brief Unix/Linux/macOS implementation of file system utilities
 */

#include "ToyFrameV/Platform/FileSystem.h"

#include <cstdlib>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace ToyFrameV {
namespace Platform {

namespace {
std::string GetHomeDirectory() {
    const char* home = std::getenv("HOME");
    return home ? home : ".";
}
} // anonymous namespace

std::string GetAssetsPath() {
    return "./assets";
}

std::string GetDocumentsPath() {
    return GetHomeDirectory() + "/.toyframev/documents";
}

std::string GetCachePath() {
    return GetHomeDirectory() + "/.toyframev/cache";
}

std::string GetTempDirectoryPath() {
    return "/tmp/toyframev";
}

bool EnsureDirectoryExists(const std::string& dirPath) {
    if (dirPath.empty()) {
        return true;
    }
    
    // Create directory recursively
    std::string path = dirPath;
    size_t pos = 0;
    
    // Handle absolute paths
    if (path[0] == '/') {
        pos = 1;
    }
    
    while ((pos = path.find('/', pos)) != std::string::npos) {
        std::string subPath = path.substr(0, pos);
        if (!subPath.empty()) {
            if (mkdir(subPath.c_str(), 0755) != 0 && errno != EEXIST) {
                return false;
            }
        }
        pos++;
    }
    
    // Create the final directory
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        return false;
    }
    
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
    // TODO: Check actual network status
    return true;
}

} // namespace Platform
} // namespace ToyFrameV
