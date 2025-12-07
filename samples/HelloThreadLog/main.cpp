#include <ToyFrameV.h>
#include <chrono>
#include <thread>
#include <vector>

using namespace ToyFrameV;
using namespace ToyFrameV::Core;

namespace {
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
constexpr bool kWebNoThreads = true;
#else
constexpr bool kWebNoThreads = false;
#endif
}

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
            Log::EnableFileSink(fileOptions);
        } else {
            TOYFRAMEV_LOG_INFO("File sink disabled on single-threaded Web build");
        }
        Log::SetLevel(Level::Debug);

        TOYFRAMEV_LOG_INFO("HelloThreadLog starting");

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
            TOYFRAMEV_LOG_INFO("All tasks finished; press ESC to exit");
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
