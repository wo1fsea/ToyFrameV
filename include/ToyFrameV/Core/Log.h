#pragma once

#include "ToyFrameV/Core/Threading.h"
#include <chrono>
#include <deque>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <fmt/core.h>

namespace ToyFrameV::Core {

enum class Level : int {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4,
    Fatal = 5
};

struct SourceLocation {
    const char* file = "";
    const char* function = "";
    int line = 0;

    static constexpr SourceLocation Current(const char* file_, const char* function_, int line_) {
        return SourceLocation{file_, function_, line_};
    }
};

struct LogMessage {
    Level level = Level::Info;
    std::string category;
    std::string text;          // Formatted message without prefix
    std::string formatted;     // Fully formatted message used by sinks
    SourceLocation location{};
    std::chrono::system_clock::time_point timestamp{};
    uint64_t threadId = 0;
};

class ILogSink {
  public:
    virtual ~ILogSink() = default;
    virtual void OnMessage(const LogMessage& message) = 0;
};

class ConsoleSink : public ILogSink {
  public:
    void OnMessage(const LogMessage& message) override;
};

class FileSink : public ILogSink {
  public:
    struct Options {
        std::string path = "logs/toyframev.log";
        size_t maxBytes = 5 * 1024 * 1024;
        size_t maxFiles = 3;
        size_t queueCapacity = 1024;
        bool flushOnShutdown = true;
    };

    explicit FileSink(Options options);
    ~FileSink() override;

    void OnMessage(const LogMessage& message) override;
    void Shutdown();

  private:
    struct Record {
        std::string formatted;
        Level level = Level::Info;
    };

    void WorkerLoop();
    void RotateIfNeeded(size_t messageSize);
    void OpenFile();
    void CloseFile();

    Options m_options;
    std::mutex m_mutex;
    std::condition_variable m_cvNotEmpty;
    std::condition_variable m_cvNotFull;
    std::deque<Record> m_queue;
    bool m_running = true;
    bool m_droppedWarningSent = false;
    std::thread m_thread;
    size_t m_currentSize = 0;
    std::ofstream m_stream;
};

class Log {
  public:
    static void SetLevel(Level level);
    static Level GetLevel();

    static void SetCategoryEnabled(std::string category, bool enabled);
    static bool IsCategoryEnabled(std::string_view category);

    static void AddSink(std::shared_ptr<ILogSink> sink);
    static void ClearSinks();
    static void UseDefaultSinks();
    static void EnableFileSink(const FileSink::Options& options);
    static void Shutdown();

    template <typename... Args>
    static void Write(Level level, std::string_view category, SourceLocation loc, std::string_view fmtStr,
                      Args&&... args) {
        if (!IsLevelEnabled(level)) {
            return;
        }
        if (!category.empty() && !IsCategoryEnabled(category)) {
            return;
        }
        std::string text = fmt::format(fmtStr, std::forward<Args>(args)...);
        Submit(level, category, loc, std::move(text));
    }

    template <typename... Args>
    static void Write(Level level, SourceLocation loc, std::string_view fmtStr, Args&&... args) {
        Write(level, {}, loc, fmtStr, std::forward<Args>(args)...);
    }

    // Convenience helpers
    template <typename... Args>
    static void Trace(std::string_view fmtStr, Args&&... args) {
        Write(Level::Trace, SourceLocation::Current(), fmtStr, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void Debug(std::string_view fmtStr, Args&&... args) {
        Write(Level::Debug, SourceLocation::Current(), fmtStr, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void Info(std::string_view fmtStr, Args&&... args) {
        Write(Level::Info, SourceLocation::Current(), fmtStr, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void Warning(std::string_view fmtStr, Args&&... args) {
        Write(Level::Warning, SourceLocation::Current(), fmtStr, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void Error(std::string_view fmtStr, Args&&... args) {
        Write(Level::Error, SourceLocation::Current(), fmtStr, std::forward<Args>(args)...);
    }
    template <typename... Args>
    static void Fatal(std::string_view fmtStr, Args&&... args) {
        Write(Level::Fatal, SourceLocation::Current(), fmtStr, std::forward<Args>(args)...);
    }

    static bool IsLevelEnabled(Level level);

  private:
    static void Submit(Level level, std::string_view category, SourceLocation loc, std::string text);
};

// Compile-time level stripping
#ifndef TOYFRAMEV_LOG_COMPILETIME_LEVEL
#if defined(NDEBUG)
#define TOYFRAMEV_LOG_COMPILETIME_LEVEL ::ToyFrameV::Core::Level::Info
#else
#define TOYFRAMEV_LOG_COMPILETIME_LEVEL ::ToyFrameV::Core::Level::Debug
#endif
#endif

#define TOYFRAMEV_DETAIL_LEVEL_VALUE(level) static_cast<int>(level)
#define TOYFRAMEV_DETAIL_COMPILETIME_LEVEL_VALUE TOYFRAMEV_DETAIL_LEVEL_VALUE(TOYFRAMEV_LOG_COMPILETIME_LEVEL)

#define TOYFRAMEV_LOG_ENABLED(level) (TOYFRAMEV_DETAIL_LEVEL_VALUE(level) >= TOYFRAMEV_DETAIL_COMPILETIME_LEVEL_VALUE)

#define TOYFRAMEV_LOG_TRACE(fmt, ...) \
    do { \
        if (TOYFRAMEV_LOG_ENABLED(::ToyFrameV::Core::Level::Trace)) { \
            ::ToyFrameV::Core::Log::Write(::ToyFrameV::Core::Level::Trace, ::ToyFrameV::Core::SourceLocation::Current(__FILE__, __func__, __LINE__), fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define TOYFRAMEV_LOG_DEBUG(fmt, ...) \
    do { \
        if (TOYFRAMEV_LOG_ENABLED(::ToyFrameV::Core::Level::Debug)) { \
            ::ToyFrameV::Core::Log::Write(::ToyFrameV::Core::Level::Debug, ::ToyFrameV::Core::SourceLocation::Current(__FILE__, __func__, __LINE__), fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define TOYFRAMEV_LOG_INFO(fmt, ...) \
    do { \
        if (TOYFRAMEV_LOG_ENABLED(::ToyFrameV::Core::Level::Info)) { \
            ::ToyFrameV::Core::Log::Write(::ToyFrameV::Core::Level::Info, ::ToyFrameV::Core::SourceLocation::Current(__FILE__, __func__, __LINE__), fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define TOYFRAMEV_LOG_WARN(fmt, ...) \
    do { \
        if (TOYFRAMEV_LOG_ENABLED(::ToyFrameV::Core::Level::Warning)) { \
            ::ToyFrameV::Core::Log::Write(::ToyFrameV::Core::Level::Warning, ::ToyFrameV::Core::SourceLocation::Current(__FILE__, __func__, __LINE__), fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define TOYFRAMEV_LOG_ERROR(fmt, ...) \
    do { \
        if (TOYFRAMEV_LOG_ENABLED(::ToyFrameV::Core::Level::Error)) { \
            ::ToyFrameV::Core::Log::Write(::ToyFrameV::Core::Level::Error, ::ToyFrameV::Core::SourceLocation::Current(__FILE__, __func__, __LINE__), fmt, ##__VA_ARGS__); \
        } \
    } while (0)

#define TOYFRAMEV_LOG_FATAL(fmt, ...) \
    do { \
        if (TOYFRAMEV_LOG_ENABLED(::ToyFrameV::Core::Level::Fatal)) { \
            ::ToyFrameV::Core::Log::Write(::ToyFrameV::Core::Level::Fatal, ::ToyFrameV::Core::SourceLocation::Current(__FILE__, __func__, __LINE__), fmt, ##__VA_ARGS__); \
        } \
    } while (0)

}  // namespace ToyFrameV::Core
