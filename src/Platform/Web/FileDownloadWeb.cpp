/**
 * @file FileDownloadWeb.cpp
 * @brief Web/Emscripten implementation of file download service
 * 
 * On WebGL, files cannot be written directly to the filesystem.
 * Instead, files are queued in memory and can be downloaded as a ZIP archive.
 */

#include "ToyFrameV/Platform/FileDownload.h"

#include <cstring>
#include <map>
#include <mutex>

#include <emscripten.h>

namespace ToyFrameV {
namespace Platform {

// Store pending files for batch ZIP download
static std::map<std::string, std::vector<uint8_t>> s_pendingFiles;
static std::mutex s_pendingMutex;

bool SaveOrQueueFile(const std::string& filename, const std::vector<uint8_t>& data) {
    if (data.empty()) return false;
    
    std::lock_guard<std::mutex> lock(s_pendingMutex);
    s_pendingFiles[filename] = data;
    return true;
}

void DownloadQueuedFiles(const std::string& zipFilename) {
    std::lock_guard<std::mutex> lock(s_pendingMutex);
    
    if (s_pendingFiles.empty()) return;
    
    // Build buffer with all files data
    // Format: [nameLen:u32, name:char[], padding, dataLen:u32, data:u8[], padding] * N
    size_t bufferSize = 0;
    for (const auto& [name, data] : s_pendingFiles) {
        bufferSize += 4 + name.size();      // nameLen + name
        bufferSize = (bufferSize + 3) & ~3; // align
        bufferSize += 4 + data.size();      // dataLen + data
        bufferSize = (bufferSize + 3) & ~3; // align
    }
    
    std::vector<uint8_t> buffer(bufferSize);
    size_t offset = 0;
    
    for (const auto& [name, data] : s_pendingFiles) {
        // Write name length
        uint32_t nameLen = static_cast<uint32_t>(name.size());
        std::memcpy(&buffer[offset], &nameLen, 4);
        offset += 4;
        
        // Write name
        std::memcpy(&buffer[offset], name.data(), name.size());
        offset += name.size();
        offset = (offset + 3) & ~3;
        
        // Write data length
        uint32_t dataLen = static_cast<uint32_t>(data.size());
        std::memcpy(&buffer[offset], &dataLen, 4);
        offset += 4;
        
        // Write data
        std::memcpy(&buffer[offset], data.data(), data.size());
        offset += data.size();
        offset = (offset + 3) & ~3;
    }
    
    int fileCount = static_cast<int>(s_pendingFiles.size());
    
    // Call JavaScript to create ZIP and download
    EM_ASM({
        // CRC32 calculation function
        var crc32Table = null;
        function makeCRC32Table() {
            var c;
            var table = [];
            for (var n = 0; n < 256; n++) {
                c = n;
                for (var k = 0; k < 8; k++) {
                    c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
                }
                table[n] = c;
            }
            return table;
        }
        function crc32(data) {
            if (!crc32Table) crc32Table = makeCRC32Table();
            var crc = 0 ^ (-1);
            for (var i = 0; i < data.length; i++) {
                crc = (crc >>> 8) ^ crc32Table[(crc ^ data[i]) & 0xFF];
            }
            return (crc ^ (-1)) >>> 0;
        }
        
        var files = [];
        var ptr = $0;
        var count = $1;
        var offset = 0;
        
        for (var i = 0; i < count; i++) {
            var nameLen = HEAPU32[(ptr + offset) >> 2];
            offset += 4;
            
            var name = "";
            for (var j = 0; j < nameLen; j++) {
                name += String.fromCharCode(HEAPU8[ptr + offset + j]);
            }
            offset += nameLen;
            offset = (offset + 3) & ~3;
            
            var dataLen = HEAPU32[(ptr + offset) >> 2];
            offset += 4;
            
            var data = new Uint8Array(dataLen);
            for (var j = 0; j < dataLen; j++) {
                data[j] = HEAPU8[ptr + offset + j];
            }
            offset += dataLen;
            offset = (offset + 3) & ~3;
            
            files.push({name: name, data: data, crc: crc32(data)});
        }
        
        // Create ZIP
        var centralDir = [];
        var localHeaders = [];
        var localOffset = 0;
        
        for (var i = 0; i < files.length; i++) {
            var file = files[i];
            var nameBytes = new TextEncoder().encode(file.name);
            
            var localHeader = new Uint8Array(30 + nameBytes.length);
            var view = new DataView(localHeader.buffer);
            view.setUint32(0, 0x04034b50, true);  // Local file header signature
            view.setUint16(4, 20, true);          // Version needed to extract
            view.setUint16(6, 0, true);           // General purpose bit flag
            view.setUint16(8, 0, true);           // Compression method (0 = stored)
            view.setUint16(10, 0, true);          // File last modification time
            view.setUint16(12, 0, true);          // File last modification date
            view.setUint32(14, file.crc, true);           // CRC-32
            view.setUint32(18, file.data.length, true);   // Compressed size
            view.setUint32(22, file.data.length, true);   // Uncompressed size
            view.setUint16(26, nameBytes.length, true);   // File name length
            view.setUint16(28, 0, true);                  // Extra field length
            localHeader.set(nameBytes, 30);
            localHeaders.push(localHeader);
            
            var cdEntry = new Uint8Array(46 + nameBytes.length);
            var cdView = new DataView(cdEntry.buffer);
            cdView.setUint32(0, 0x02014b50, true);   // Central directory signature
            cdView.setUint16(4, 20, true);           // Version made by
            cdView.setUint16(6, 20, true);           // Version needed to extract
            cdView.setUint16(8, 0, true);            // General purpose bit flag
            cdView.setUint16(10, 0, true);           // Compression method
            cdView.setUint16(12, 0, true);           // File last modification time
            cdView.setUint16(14, 0, true);           // File last modification date
            cdView.setUint32(16, file.crc, true);            // CRC-32
            cdView.setUint32(20, file.data.length, true);    // Compressed size
            cdView.setUint32(24, file.data.length, true);    // Uncompressed size
            cdView.setUint16(28, nameBytes.length, true);    // File name length
            cdView.setUint16(30, 0, true);                   // Extra field length
            cdView.setUint16(32, 0, true);                   // File comment length
            cdView.setUint16(34, 0, true);                   // Disk number start
            cdView.setUint16(36, 0, true);                   // Internal file attributes
            cdView.setUint32(38, 0, true);                   // External file attributes
            cdView.setUint32(42, localOffset, true);         // Relative offset of local header
            cdEntry.set(nameBytes, 46);
            centralDir.push(cdEntry);
            
            localOffset += localHeader.length + file.data.length;
        }
        
        var totalSize = localOffset;
        var cdOffset = totalSize;
        var cdSize = 0;
        for (var i = 0; i < centralDir.length; i++) {
            cdSize += centralDir[i].length;
        }
        totalSize += cdSize + 22;
        
        var zip = new Uint8Array(totalSize);
        var zipOffset = 0;
        
        for (var i = 0; i < files.length; i++) {
            zip.set(localHeaders[i], zipOffset);
            zipOffset += localHeaders[i].length;
            zip.set(files[i].data, zipOffset);
            zipOffset += files[i].data.length;
        }
        
        for (var i = 0; i < centralDir.length; i++) {
            zip.set(centralDir[i], zipOffset);
            zipOffset += centralDir[i].length;
        }
        
        var eocd = new Uint8Array(22);
        var eocdView = new DataView(eocd.buffer);
        eocdView.setUint32(0, 0x06054b50, true);   // End of central directory signature
        eocdView.setUint16(4, 0, true);            // Number of this disk
        eocdView.setUint16(6, 0, true);            // Disk where central directory starts
        eocdView.setUint16(8, files.length, true); // Number of central directory records on this disk
        eocdView.setUint16(10, files.length, true); // Total number of central directory records
        eocdView.setUint32(12, cdSize, true);      // Size of central directory
        eocdView.setUint32(16, cdOffset, true);    // Offset of start of central directory
        eocdView.setUint16(20, 0, true);           // Comment length
        zip.set(eocd, zipOffset);
        
        // Use the global download function defined in template.html
        var filename = UTF8ToString($2);
        if (window.ToyFrameV_DownloadBlob) {
            window.ToyFrameV_DownloadBlob(zip, filename);
        } else {
            console.error('ToyFrameV_DownloadBlob not found! Fallback to direct download.');
            var blob = new Blob([zip], {type: 'application/octet-stream'});
            var url = URL.createObjectURL(blob);
            var a = document.createElement('a');
            a.href = url;
            a.download = filename;
            document.body.appendChild(a);
            a.click();
            document.body.removeChild(a);
            URL.revokeObjectURL(url);
        }
    }, buffer.data(), fileCount, zipFilename.c_str());
    
    s_pendingFiles.clear();
}

size_t GetQueuedFileCount() {
    std::lock_guard<std::mutex> lock(s_pendingMutex);
    return s_pendingFiles.size();
}

void ClearQueuedFiles() {
    std::lock_guard<std::mutex> lock(s_pendingMutex);
    s_pendingFiles.clear();
}

} // namespace Platform
} // namespace ToyFrameV
