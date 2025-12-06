/**
 * @file main.cpp
 * @brief HelloIO - IOSystem demonstration sample
 *
 * This sample demonstrates how to use the IOSystem for file I/O:
 * - Reading and writing local files
 * - Using different path schemes (assets://, documents://, etc.)
 * - Asynchronous file operations
 * - IOBuffer usage and zero-copy patterns
 */

#include <ToyFrameV.h>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace ToyFrameV;

class HelloIOApp : public App {
  public:
    HelloIOApp() {
        m_config.title = "Hello IOSystem";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
    }

  protected:
    bool OnInit() override {
        std::cout << "========================================" << std::endl;
        std::cout << "    HelloIO - IOSystem Demo" << std::endl;
        std::cout << "========================================" << std::endl;

        // Get IOSystem reference
        auto *io = GetSystem<IOSystem>();
        if (!io) {
            std::cerr << "IOSystem not available!" << std::endl;
            return false;
        }

        // Display platform paths
        std::cout << "\n--- Platform Paths ---" << std::endl;
        std::cout << "Assets:    " << io->GetAssetsPath() << std::endl;
        std::cout << "Documents: " << io->GetDocumentsPath() << std::endl;
        std::cout << "Cache:     " << io->GetCachePath() << std::endl;
        std::cout << "Temp:      " << io->GetTempPath() << std::endl;

        // Test 1: Write a text file
        std::cout << "\n--- Test 1: Write Text File ---" << std::endl;
        {
            std::string content = "Hello from ToyFrameV IOSystem!\n";
            content += "This file was created at runtime.\n";
            content += "Timestamp: " + GetTimestamp() + "\n";

            IOResult result = io->WriteTextFile("documents://test/hello.txt", content);
            if (result.IsSuccess()) {
                std::cout << "Successfully wrote: documents://test/hello.txt" << std::endl;
                std::cout << "Resolved path: " << io->ResolvePath("documents://test/hello.txt")
                          << std::endl;
            } else {
                std::cout << "Write failed: " << result.errorMessage << std::endl;
            }
        }

        // Test 2: Read the file back
        std::cout << "\n--- Test 2: Read Text File ---" << std::endl;
        {
            std::string content = io->ReadTextFile("documents://test/hello.txt");
            if (!content.empty()) {
                std::cout << "File contents:\n" << content << std::endl;
            } else {
                std::cout << "Failed to read file" << std::endl;
            }
        }

        // Test 3: Write binary data
        std::cout << "\n--- Test 3: Write Binary File ---" << std::endl;
        {
            // Create some binary data
            std::vector<uint8_t> binaryData;
            for (int i = 0; i < 256; ++i) {
                binaryData.push_back(static_cast<uint8_t>(i));
            }

            IOResult result =
                io->WriteFile("documents://test/binary.dat", binaryData.data(), binaryData.size());
            if (result.IsSuccess()) {
                std::cout << "Successfully wrote " << binaryData.size() << " bytes to binary.dat"
                          << std::endl;
            } else {
                std::cout << "Write failed: " << result.errorMessage << std::endl;
            }
        }

        // Test 4: Read binary data back
        std::cout << "\n--- Test 4: Read Binary File ---" << std::endl;
        {
            IOResult result = io->ReadFile("documents://test/binary.dat");
            if (result.IsSuccess()) {
                std::cout << "Read " << result.Size() << " bytes" << std::endl;

                // Display first 16 bytes as hex
                std::cout << "First 16 bytes: ";
                for (size_t i = 0; i < std::min(result.Size(), size_t(16)); ++i) {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << static_cast<int>(result.Data()[i]) << " ";
                }
                std::cout << std::dec << std::endl;
            } else {
                std::cout << "Read failed: " << result.errorMessage << std::endl;
            }
        }

        // Test 5: IOBuffer usage
        std::cout << "\n--- Test 5: IOBuffer Demo ---" << std::endl;
        {
            // Create buffer from string
            std::string testStr = "Buffer test data";
            IOBuffer buffer(testStr.data(), testStr.size());

            std::cout << "Buffer size: " << buffer.Size() << std::endl;
            std::cout << "As string_view: " << buffer.AsStringView() << std::endl;
            std::cout << "ToString: " << buffer.ToString() << std::endl;

            // Move buffer (zero-copy)
            IOBuffer moved = std::move(buffer);
            std::cout << "After move - original empty: " << (buffer.Empty() ? "yes" : "no")
                      << std::endl;
            std::cout << "After move - moved size: " << moved.Size() << std::endl;
        }

        // Test 6: File existence check
        std::cout << "\n--- Test 6: File Existence ---" << std::endl;
        {
            std::cout << "documents://test/hello.txt exists: "
                      << (io->Exists("documents://test/hello.txt") ? "yes" : "no") << std::endl;
            std::cout << "documents://test/nonexistent.txt exists: "
                      << (io->Exists("documents://test/nonexistent.txt") ? "yes" : "no")
                      << std::endl;
        }

        // Test 7: Get file size
        std::cout << "\n--- Test 7: File Size ---" << std::endl;
        {
            size_t size = io->GetFileSize("documents://test/binary.dat");
            std::cout << "binary.dat size: " << size << " bytes" << std::endl;
        }

        // Test 8: Async read (callback will be called in Update)
        std::cout << "\n--- Test 8: Async Read (callback in Update) ---" << std::endl;
        {
            m_asyncRequest =
                io->ReadFileAsync("documents://test/hello.txt", [this](IOResult result) {
                    m_asyncCompleted = true;
                    if (result.IsSuccess()) {
                        std::cout << "[Async Callback] Read " << result.Size() << " bytes"
                                  << std::endl;
                        std::cout << "[Async Callback] Content: " << result.AsStringView()
                                  << std::endl;
                    } else {
                        std::cout << "[Async Callback] Error: " << result.errorMessage << std::endl;
                    }
                });
            std::cout << "Async request initiated, waiting for callback..." << std::endl;
        }

        // Test 9: Delete file
        std::cout << "\n--- Test 9: Delete File ---" << std::endl;
        {
            // Write a temp file first
            io->WriteTextFile("temp://to_delete.txt", "This will be deleted");
            std::cout << "temp://to_delete.txt exists: "
                      << (io->Exists("temp://to_delete.txt") ? "yes" : "no") << std::endl;

            bool deleted = io->Delete("temp://to_delete.txt");
            std::cout << "Deleted: " << (deleted ? "yes" : "no") << std::endl;
            std::cout << "temp://to_delete.txt exists after delete: "
                      << (io->Exists("temp://to_delete.txt") ? "yes" : "no") << std::endl;
        }

        std::cout << "\n========================================" << std::endl;
        std::cout << "Press ESC to exit" << std::endl;
        std::cout << "========================================" << std::endl;

        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Check for ESC to quit
        if (Input::IsKeyPressed(KeyCode::Escape)) {
            Quit();
        }

        // Report when async completes
        if (m_asyncCompleted && !m_asyncReported) {
            m_asyncReported = true;
            std::cout << "\n[Main Thread] Async operation completed!" << std::endl;
        }
    }

    void OnRender() override {
        // Clear to a nice color
        GetGraphics()->Clear(Color(0.2f, 0.3f, 0.4f));
    }

    void OnShutdown() override {
        std::cout << "\nHelloIO shutdown!" << std::endl;

        // Clean up test files
        auto *io = GetSystem<IOSystem>();
        if (io) {
            io->Delete("documents://test/hello.txt");
            io->Delete("documents://test/binary.dat");
            std::cout << "Cleaned up test files" << std::endl;
        }
    }

  private:
    std::string GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }

    IORequestPtr m_asyncRequest;
    bool m_asyncCompleted = false;
    bool m_asyncReported = false;
};

// Entry point
TOYFRAMEV_MAIN(HelloIOApp)
