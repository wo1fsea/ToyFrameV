/**
 * @file FileSystemWindows.cpp
 * @brief Windows implementation of file system utilities
 */

#include "ToyFrameV/Platform/FileSystem.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <ShlObj.h>
#include <Windows.h>
#pragma comment(lib, "Shell32.lib")

namespace ToyFrameV {
namespace Platform {

std::string GetAssetsPath() {
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::string exeDir(exePath);
    size_t lastSlash = exeDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }
    return exeDir + "\\assets";
}

std::string GetDocumentsPath() {
    char docPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, docPath))) {
        return std::string(docPath) + "\\ToyFrameV";
    }
    // Fallback to exe directory
    return GetAssetsPath().substr(0, GetAssetsPath().length() - 7) + "\\documents";
}

std::string GetCachePath() {
    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath)) {
        return std::string(tempPath) + "ToyFrameV\\cache";
    }
    return GetAssetsPath().substr(0, GetAssetsPath().length() - 7) + "\\cache";
}

std::string GetTempDirectoryPath() {
    char tempPath[MAX_PATH];
    if (::GetTempPathA(MAX_PATH, tempPath)) {
        return std::string(tempPath) + "ToyFrameV";
    }
    return GetAssetsPath().substr(0, GetAssetsPath().length() - 7) + "\\temp";
}

bool EnsureDirectoryExists(const std::string& dirPath) {
    if (dirPath.empty()) {
        return true;
    }
    
    // SHCreateDirectoryExA creates all intermediate directories
    int result = SHCreateDirectoryExA(nullptr, dirPath.c_str(), nullptr);
    return result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS;
}

std::string NormalizePath(const std::string& path) {
    std::string result = path;
    for (char& c : result) {
        if (c == '/') {
            c = '\\';
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
    if (!result.empty() && (result.back() == '\\' || result.back() == '/')) {
        result.pop_back();
    }
    // Remove leading separator from relative
    std::string rel = relative;
    if (!rel.empty() && (rel.front() == '\\' || rel.front() == '/')) {
        rel = rel.substr(1);
    }
    
    return NormalizePath(result + "\\" + rel);
}

bool IsNetworkAvailable() {
    // TODO: Check actual network status using WinINet or similar
    return true;
}

} // namespace Platform
} // namespace ToyFrameV
