#pragma once

/**
 * @file FileSystem.h
 * @brief Platform-abstracted file system utilities
 * 
 * Provides platform-independent access to:
 * - Standard directory paths (assets, documents, cache, temp)
 * - Directory creation
 * - Path manipulation
 */

#include "ToyFrameV/Platform.h"
#include <string>

namespace ToyFrameV {
namespace Platform {

/**
 * @brief Platform path separator character
 */
#ifdef PLATFORM_WINDOWS
constexpr char PathSeparator = '\\';
constexpr char AltPathSeparator = '/';
#else
constexpr char PathSeparator = '/';
constexpr char AltPathSeparator = '\\';
#endif

/**
 * @brief Get the assets directory path
 * @return Path to the assets directory
 * 
 * - Windows: <exe_dir>\assets
 * - Web: /assets
 * - Unix: ./assets
 */
std::string GetAssetsPath();

/**
 * @brief Get the documents directory path
 * @return Path to the user documents directory
 * 
 * - Windows: %USERPROFILE%\Documents\ToyFrameV
 * - Web: /home/web_user
 * - Unix: ~/.toyframev/documents
 */
std::string GetDocumentsPath();

/**
 * @brief Get the cache directory path
 * @return Path to the cache directory
 * 
 * - Windows: %TEMP%\ToyFrameV\cache
 * - Web: /tmp/cache
 * - Unix: ~/.toyframev/cache
 */
std::string GetCachePath();

/**
 * @brief Get the temp directory path
 * @return Path to the temporary files directory
 * 
 * - Windows: %TEMP%\ToyFrameV
 * - Web: /tmp
 * - Unix: /tmp/toyframev
 */
std::string GetTempDirectoryPath();

/**
 * @brief Ensure a directory exists, creating it if necessary
 * @param dirPath Path to the directory to create
 * @return true if directory exists or was created successfully
 * 
 * Creates all intermediate directories as needed (like mkdir -p).
 */
bool EnsureDirectoryExists(const std::string& dirPath);

/**
 * @brief Normalize path separators for the current platform
 * @param path Path to normalize
 * @return Path with correct separators for current platform
 */
std::string NormalizePath(const std::string& path);

/**
 * @brief Join two path components with the correct separator
 * @param base Base path
 * @param relative Relative path to append
 * @return Combined path
 */
std::string JoinPath(const std::string& base, const std::string& relative);

/**
 * @brief Check if the platform has network capabilities
 * @return true if network I/O is available
 */
bool IsNetworkAvailable();

} // namespace Platform
} // namespace ToyFrameV
