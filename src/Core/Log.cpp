#include "ToyFrameV/Core/Log.h"
#include "ToyFrameV/Core/Threading.h"
#include <chrono>
#include <cstdio>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>

#ifdef PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace ToyFrameV::Core {

namespace {

std::string LevelToString(Level level) {
    switch (level) {
        case Level::Trace: return "Trace";
        case Level::Debug: return "Debug";
        case Level::Info: return "Info";
        case Level::Warning: return "Warning";
        case Level::Error: return "Error";
        case Level::Fatal: return "Fatal";
    }
    return "Unknown";
}

uint64_t GetThreadId() {
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

std::string FormatTimestamp(const std::chrono::system_clock::time_point& tp) {
    auto time = std::chrono::system_clock::to_time_t(tp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) % 1000;
    std::tm tm {};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << '.'
        << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

std::string BuildFormatted(const LogMessage& msg) {
    std::ostringstream oss;
    oss << '[' << FormatTimestamp(msg.timestamp) << ']';
    oss << "[tid:" << msg.threadId << ']';
    oss << '[' << LevelToString(msg.level) << ']';
    if (!msg.category.empty()) {
        oss << '[' << msg.category << ']';
    }
    oss << ' ' << msg.text;
    return oss.str();
}

class LoggerInstance {
  public:
    LoggerInstance() {
        // Default: add console sink
        sinks.push_back(std::make_shared<ConsoleSink>());
    }

    std::mutex mutex;
    Level runtimeLevel = Level::Debug;
    std::vector<std::shared_ptr<ILogSink>> sinks;
    std::unordered_map<std::string, bool> categoryEnabled;
};

LoggerInstance& Instance() {
    static LoggerInstance inst;
    return inst;
}

}  // namespace

// ============================== ConsoleSink =================================

void ConsoleSink::OnMessage(const LogMessage& message) {
#ifdef __EMSCRIPTEN__
    std::string line = message.formatted;
    if (message.level >= Level::Error) {
        EM_ASM_({ console.error(UTF8ToString($0)); }, line.c_str());
    } else {
        EM_ASM_({ console.log(UTF8ToString($0)); }, line.c_str());
    }
    return;
#else
#ifdef PLATFORM_WINDOWS
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info {};
    GetConsoleScreenBufferInfo(hConsole, &info);
    WORD originalAttr = info.wAttributes;

    WORD color = originalAttr;
    switch (message.level) {
        case Level::Trace:
        case Level::Debug: color = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case Level::Info: color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case Level::Warning: color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case Level::Error:
        case Level::Fatal: color = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
    }
    SetConsoleTextAttribute(hConsole, color);
    std::cout << message.formatted << std::endl;
    SetConsoleTextAttribute(hConsole, originalAttr);
#else
    const char* color = "";
    const char* reset = "\033[0m";
    switch (message.level) {
        case Level::Trace:
        case Level::Debug: color = "\033[36m"; break;
        case Level::Info: color = "\033[37m"; break;
        case Level::Warning: color = "\033[33m"; break;
        case Level::Error:
        case Level::Fatal: color = "\033[31m"; break;
    }
    std::cout << color << message.formatted << reset << std::endl;
#endif
#endif
}

// ============================== FileSink ====================================

FileSink::FileSink(Options options) : m_options(std::move(options)) {
    OpenFile();
    m_thread = std::thread([this] { WorkerLoop(); });
}

FileSink::~FileSink() { Shutdown(); }

void FileSink::Shutdown() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running) return;
        m_running = false;
    }
    m_cvNotEmpty.notify_all();
    m_cvNotFull.notify_all();
    if (m_thread.joinable()) {
        m_thread.join();
    }
    CloseFile();
}

void FileSink::OnMessage(const LogMessage& message) {
    Record rec;
    rec.formatted = message.formatted;
    rec.level = message.level;

    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvNotFull.wait(lock, [&] { return !m_running || m_queue.size() < m_options.queueCapacity; });
    if (!m_running) return;
    m_queue.emplace_back(std::move(rec));
    m_cvNotEmpty.notify_one();
}

void FileSink::WorkerLoop() {
    while (true) {
        Record rec;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cvNotEmpty.wait(lock, [&] { return !m_running || !m_queue.empty(); });
            if (!m_running && m_queue.empty()) {
                break;
            }
            rec = std::move(m_queue.front());
            m_queue.pop_front();
            m_cvNotFull.notify_one();
        }

        if (!m_stream.is_open()) {
            OpenFile();
        }
        if (!m_stream.is_open()) {
            continue;
        }

        RotateIfNeeded(rec.formatted.size() + 1);
        m_stream << rec.formatted << '\n';
        m_currentSize += rec.formatted.size() + 1;
        if (rec.level == Level::Fatal || m_options.flushOnShutdown) {
            m_stream.flush();
        }
    }

    if (m_options.flushOnShutdown && m_stream.is_open()) {
        m_stream.flush();
    }
}

void FileSink::OpenFile() {
    std::filesystem::path p = m_options.path;
    if (p.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(p.parent_path(), ec);
    }
    m_stream.open(m_options.path, std::ios::app);
    if (m_stream.good()) {
        m_stream.seekp(0, std::ios::end);
        m_currentSize = static_cast<size_t>(m_stream.tellp());
    } else {
        m_currentSize = 0;
    }
}

void FileSink::CloseFile() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
}

void FileSink::RotateIfNeeded(size_t messageSize) {
    if (m_options.maxBytes == 0 || !m_stream.is_open()) {
        return;
    }
    if (m_options.maxFiles == 0) {
        return;
    }
    if (m_currentSize + messageSize < m_options.maxBytes) {
        return;
    }

    CloseFile();

    // Rotate existing files: file.log.N
    for (size_t i = m_options.maxFiles; i-- > 0;) {
        std::filesystem::path oldName = m_options.path;
        if (i > 0) {
            oldName += "." + std::to_string(i);
        }
        std::filesystem::path newName = m_options.path;
        newName += "." + std::to_string(i + 1);

        std::error_code ec;
        if (std::filesystem::exists(oldName, ec)) {
            std::filesystem::rename(oldName, newName, ec);
        }
    }

    OpenFile();
    m_currentSize = 0;
}

// ================================ Log =======================================

void Log::SetLevel(Level level) {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.runtimeLevel = level;
}

Level Log::GetLevel() {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    return inst.runtimeLevel;
}

void Log::SetCategoryEnabled(std::string category, bool enabled) {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.categoryEnabled[std::move(category)] = enabled;
}

bool Log::IsCategoryEnabled(std::string_view category) {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    if (category.empty()) return true;
    auto it = inst.categoryEnabled.find(std::string(category));
    if (it == inst.categoryEnabled.end()) return true;
    return it->second;
}

void Log::AddSink(std::shared_ptr<ILogSink> sink) {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.sinks.push_back(std::move(sink));
}

void Log::ClearSinks() {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.sinks.clear();
}

void Log::UseDefaultSinks() {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    inst.sinks.clear();
    inst.sinks.push_back(std::make_shared<ConsoleSink>());
}

void Log::EnableFileSink(const FileSink::Options& options) {
    AddSink(std::make_shared<FileSink>(options));
}

void Log::Shutdown() {
    auto& inst = Instance();
    std::vector<std::shared_ptr<ILogSink>> sinksCopy;
    {
        std::lock_guard<std::mutex> lock(inst.mutex);
        sinksCopy = inst.sinks;
        inst.sinks.clear();
    }
    for (auto& sink : sinksCopy) {
        if (auto fileSink = std::dynamic_pointer_cast<FileSink>(sink)) {
            fileSink->Shutdown();
        }
    }
}

bool Log::IsLevelEnabled(Level level) {
    auto& inst = Instance();
    std::lock_guard<std::mutex> lock(inst.mutex);
    return static_cast<int>(level) >= static_cast<int>(inst.runtimeLevel);
}

void Log::Submit(Level level, std::string_view category, SourceLocation loc, std::string text) {
    LogMessage msg;
    msg.level = level;
    msg.category = std::string(category);
    msg.location = loc;
    msg.text = std::move(text);
    msg.timestamp = std::chrono::system_clock::now();
    msg.threadId = GetThreadId();
    msg.formatted = BuildFormatted(msg);

    auto& inst = Instance();
    std::vector<std::shared_ptr<ILogSink>> sinksCopy;
    {
        std::lock_guard<std::mutex> lock(inst.mutex);
        sinksCopy = inst.sinks;
    }

    for (auto& sink : sinksCopy) {
        if (sink) {
            sink->OnMessage(msg);
        }
    }
}

}  // namespace ToyFrameV::Core
