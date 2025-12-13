#include <ToyFrameV.h>
#include <chrono>
#include <thread>
#include <vector>

using namespace ToyFrameV;
using namespace ToyFrameV::Core;

namespace {
// Use PLATFORM_WEB macro from Platform.h for consistency
#if defined(PLATFORM_WEB) && !defined(__EMSCRIPTEN_PTHREADS__)
constexpr bool kWebNoThreads = true;
#else
constexpr bool kWebNoThreads = false;
#endif

// Test all log levels
void TestLogLevels() {
  TOYFRAMEV_LOG_TRACE("This is a TRACE message");
  TOYFRAMEV_LOG_DEBUG("This is a DEBUG message");
  TOYFRAMEV_LOG_INFO("This is an INFO message");
  TOYFRAMEV_LOG_WARN("This is a WARNING message");
  TOYFRAMEV_LOG_ERROR("This is an ERROR message");
  // Skip FATAL in normal test as it might have special handling
}

// Test format string edge cases
void TestFormatEdgeCases() {
  TOYFRAMEV_LOG_INFO("=== Format Edge Cases ===");

  // Empty message
  TOYFRAMEV_LOG_INFO("");

  // No placeholders
  TOYFRAMEV_LOG_INFO("Simple message with no placeholders");

  // Multiple placeholders
  TOYFRAMEV_LOG_INFO("Multiple: {} {} {} {}", 1, 2, 3, 4);

  // Different types
  TOYFRAMEV_LOG_INFO("Int: {}, Float: {}, String: {}, Bool: {}", 42, 3.14159,
                     "hello", true);

  // Escaped braces {{ and }}
  TOYFRAMEV_LOG_INFO("Escaped braces: {{literal}} and value: {}", 123);
  TOYFRAMEV_LOG_INFO("Double escape: {{{{nested}}}} with {}", "arg");

  // Long string
  std::string longStr(200, 'A');
  TOYFRAMEV_LOG_INFO("Long string: {}", longStr);

  // Special characters
  TOYFRAMEV_LOG_INFO("Special chars: tab[\\t] quote[\\\"] backslash[\\\\]");

  // Unicode (if supported)
  TOYFRAMEV_LOG_INFO("Unicode test: Chinese, Emoji, Japanese");

  // Negative numbers
  TOYFRAMEV_LOG_INFO("Negative: {} {}", -42, -3.14);

  // Zero and edge numeric values
  TOYFRAMEV_LOG_INFO("Edge numbers: {} {} {}", 0, 0.0, -0.0);

  TOYFRAMEV_LOG_INFO("=== Format Edge Cases Done ===");
}

// Test log level filtering
void TestLevelFiltering() {
  TOYFRAMEV_LOG_INFO("=== Level Filtering Test ===");

  // Set to Warning level - should filter out Trace, Debug, Info
  Log::SetLevel(Level::Warning);
  TOYFRAMEV_LOG_DEBUG("This DEBUG should NOT appear");
  TOYFRAMEV_LOG_INFO("This INFO should NOT appear");
  TOYFRAMEV_LOG_WARN("This WARNING should appear");
  TOYFRAMEV_LOG_ERROR("This ERROR should appear");

  // Restore to Debug level
  Log::SetLevel(Level::Debug);
  TOYFRAMEV_LOG_DEBUG("Level restored - DEBUG visible again");

  TOYFRAMEV_LOG_INFO("=== Level Filtering Test Done ===");
}

// Test category filtering
void TestCategoryFiltering() {
  TOYFRAMEV_LOG_INFO("=== Category Filtering Test ===");

  // Note: Category filtering requires using Write() directly with category
  // For now just demonstrate the API exists
  Log::SetCategoryEnabled("TestCategory", false);
  bool enabled = Log::IsCategoryEnabled("TestCategory");
  TOYFRAMEV_LOG_INFO("Category 'TestCategory' enabled: {}", enabled);

  Log::SetCategoryEnabled("TestCategory", true);
  enabled = Log::IsCategoryEnabled("TestCategory");
  TOYFRAMEV_LOG_INFO("Category 'TestCategory' enabled after re-enable: {}",
                     enabled);

  // Unknown category should default to enabled
  enabled = Log::IsCategoryEnabled("UnknownCategory");
  TOYFRAMEV_LOG_INFO("Unknown category enabled (default): {}", enabled);

  TOYFRAMEV_LOG_INFO("=== Category Filtering Test Done ===");
}

// Test rapid logging (stress test)
void TestRapidLogging() {
  TOYFRAMEV_LOG_INFO("=== Rapid Logging Test (100 messages) ===");
  for (int i = 0; i < 100; ++i) {
    TOYFRAMEV_LOG_DEBUG("Rapid message {}", i);
  }
  TOYFRAMEV_LOG_INFO("=== Rapid Logging Test Done ===");
}

} // namespace

class HelloThreadLogApp : public App {
  public:
    HelloThreadLogApp() {
        m_config.title = "Hello Thread & Log";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
    }

  protected:
    bool OnInit() override {
        // Configure logging: console (sync) + file (async)
        Log::UseDefaultSinks();
        if (!kWebNoThreads) {  // Skip file sink on single-threaded Web build
            FileSink::Options fileOptions;
            fileOptions.path = "logs/hello_thread_log.log";
            fileOptions.queueCapacity = 64;  // small to demonstrate backpressure
            fileOptions.flushEachMessage = false; // Better performance
            Log::EnableFileSink(fileOptions);
        } else {
            TOYFRAMEV_LOG_INFO("File sink disabled on single-threaded Web build");
        }
        Log::SetLevel(Level::Debug);

        TOYFRAMEV_LOG_INFO("========================================");
        TOYFRAMEV_LOG_INFO("  HelloThreadLog - Log System Test");
        TOYFRAMEV_LOG_INFO("========================================");

        // Run edge case tests
        TestLogLevels();
        TestFormatEdgeCases();
        TestLevelFiltering();
        TestCategoryFiltering();
        TestRapidLogging();

        TOYFRAMEV_LOG_INFO("========================================");
        TOYFRAMEV_LOG_INFO("  ThreadPool Test");
        TOYFRAMEV_LOG_INFO("========================================");

        // Kick off some tasks
        auto& pool = ThreadPool::GetDefault(4, 64);
        for (int i = 0; i < 16; ++i) {
            auto future = pool.Submit([i] {
                TOYFRAMEV_LOG_DEBUG("Task {} started", i);
                if constexpr (kWebNoThreads) {
                    // On single-threaded Web builds, avoid blocking sleeps
                    volatile int acc = 0;
                    for (int j = 0; j < 10000 + i * 1000; ++j) {
                        acc += j % (i + 1);
                    }
                    return acc;
                } else {
                    std::this_thread::sleep_for(std::chrono::milliseconds(50 + (i * 10)));
                    return i * i;
                }
            });
            m_tasks.push_back({i, std::move(future), false});
        }

        TOYFRAMEV_LOG_INFO("Submitted {} tasks to ThreadPool", m_tasks.size());
        return true;
    }

    void OnUpdate(float) override {
        // Quit on ESC
        if (Input::IsKeyPressed(KeyCode::Escape)) {
            Quit();
        }

        // Poll task results
        for (auto& task : m_tasks) {
            if (task.reported) continue;
            if (task.future.IsReady()) {
                try {
                    auto value = task.future.Get();
                    TOYFRAMEV_LOG_INFO("Task {} completed with result {}", task.index, value);
                } catch (const std::exception& ex) {
                    TOYFRAMEV_LOG_ERROR("Task {} failed: {}", task.index, ex.what());
                }
                task.reported = true;
            }
        }

        if (AllTasksReported() && !m_allReportedLogged) {
            m_allReportedLogged = true;
            TOYFRAMEV_LOG_INFO("========================================");
            TOYFRAMEV_LOG_INFO("All tasks finished; press ESC to exit");
            TOYFRAMEV_LOG_INFO("========================================");
        }
    }

    void OnRender() override {
        // Soft background; actual rendering not focus of this sample
        GetGraphics()->Clear(Color(0.15f, 0.18f, 0.22f));
    }

    void OnShutdown() override {
        TOYFRAMEV_LOG_INFO("Shutting down HelloThreadLog");
        Log::Shutdown();
    }

  private:
    struct TaskInfo {
        int index;
        Future<int> future;
        bool reported = false;
    };

    bool AllTasksReported() const {
        for (const auto& t : m_tasks) {
            if (!t.reported) return false;
        }
        return true;
    }

    std::vector<TaskInfo> m_tasks;
    bool m_allReportedLogged = false;
};

TOYFRAMEV_MAIN(HelloThreadLogApp)
