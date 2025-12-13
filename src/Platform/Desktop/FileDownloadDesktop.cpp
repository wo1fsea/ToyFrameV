/**
 * @file FileDownloadDesktop.cpp
 * @brief Desktop platform implementation of file download service
 * 
 * On Desktop platforms, files are written directly to the filesystem.
 * The queued download functions are no-ops since files are saved immediately.
 */

#include "ToyFrameV/Platform/FileDownload.h"

#include <fstream>

namespace ToyFrameV {
namespace Platform {

bool SaveOrQueueFile(const std::string& filename, const std::vector<uint8_t>& data) {
    if (data.empty()) return false;
    
    // Write directly to file on Desktop
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    
    file.write(reinterpret_cast<const char*>(data.data()), 
               static_cast<std::streamsize>(data.size()));
    return file.good();
}

void DownloadQueuedFiles(const std::string& /*zipFilename*/) {
    // No-op on Desktop - files are already saved to filesystem
}

size_t GetQueuedFileCount() {
    // Always 0 on Desktop - files are saved immediately, not queued
    return 0;
}

void ClearQueuedFiles() {
    // No-op on Desktop - no queue to clear
}

} // namespace Platform
} // namespace ToyFrameV
