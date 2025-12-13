#pragma once

/**
 * @file FileDownload.h
 * @brief Platform-abstracted file download service
 * 
 * On Desktop platforms, files are written directly to the filesystem.
 * On Web platforms (WebGL), files are queued and can be downloaded as a ZIP.
 */

#include "ToyFrameV/Platform.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ToyFrameV {
namespace Platform {

/**
 * @brief Queue binary data for download (Web) or save directly (Desktop)
 * @param filename The filename to save/queue
 * @param data The binary data to save
 * @return true on success
 * 
 * On Desktop: Writes directly to file system
 * On Web: Adds to internal queue for batch download
 */
bool SaveOrQueueFile(const std::string& filename, const std::vector<uint8_t>& data);

/**
 * @brief Download all queued files as a ZIP archive
 * @param zipFilename Name of the ZIP file to download
 * 
 * On Desktop: No-op (files are already saved)
 * On Web: Creates ZIP from queued files and triggers browser download
 */
void DownloadQueuedFiles(const std::string& zipFilename = "files.zip");

/**
 * @brief Get the number of files in the download queue
 * @return Number of queued files (always 0 on Desktop)
 */
size_t GetQueuedFileCount();

/**
 * @brief Clear all queued files without downloading
 * 
 * On Desktop: No-op
 * On Web: Clears the pending file queue
 */
void ClearQueuedFiles();

/**
 * @brief Check if the platform uses queued downloads
 * @return true on Web platforms, false on Desktop
 */
constexpr bool UsesQueuedDownloads() {
#ifdef PLATFORM_WEB
    return true;
#else
    return false;
#endif
}

} // namespace Platform
} // namespace ToyFrameV
