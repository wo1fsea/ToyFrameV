#include "ToyFrameV/Core/Threading.h"
#include <algorithm>
#include <stdexcept>

namespace ToyFrameV::Core {

ThreadPool::ThreadPool(size_t threadCount, size_t maxQueueSize)
    : m_threadCount(threadCount == 0 ? std::max<size_t>(1, std::thread::hardware_concurrency()) : threadCount),
      m_maxQueueSize(maxQueueSize == 0 ? 1 : maxQueueSize) {
#if !(defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__))
    m_workers.reserve(m_threadCount);
    for (size_t i = 0; i < m_threadCount; ++i) {
        m_workers.emplace_back([this] { WorkerLoop(); });
    }
#endif
}

ThreadPool::~ThreadPool() { Shutdown(true); }

bool ThreadPool::Dequeue(Task& out) {
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cvTasks.wait(lock, [&] { return m_stopping || !m_tasks.empty(); });
    if (m_stopping && m_tasks.empty()) {
        return false;
    }
    out = std::move(m_tasks.front());
    m_tasks.pop_front();
    m_cvSpace.notify_one();
    return true;
}

void ThreadPool::WorkerLoop() {
    while (true) {
        Task task;
        if (!Dequeue(task)) {
            return;
        }
        if (task.fn) {
            task.fn();
        }
    }
}

void ThreadPool::CancelAllPending() {
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& task : m_tasks) {
        if (task.state) {
            task.state->TryCancel();
        }
    }
    m_tasks.clear();
    m_cvSpace.notify_all();
}

void ThreadPool::Shutdown(bool wait) {
#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
    // Immediate mode: nothing to do
    return;
#else
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stopping) return;
        m_stopping = true;
        if (!wait) {
            for (auto& task : m_tasks) {
                if (task.state) {
                    task.state->TryCancel();
                }
            }
            m_tasks.clear();
        }
    }
    m_cvTasks.notify_all();
    m_cvSpace.notify_all();

    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
#endif
}

size_t ThreadPool::GetQueueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

ThreadPool& ThreadPool::GetDefault(size_t threads, size_t maxQueueSize) {
  // Note: Parameters are only used on first call. Subsequent calls return the
  // existing pool.
  static ThreadPool defaultPool(threads, maxQueueSize);
  return defaultPool;
}

}  // namespace ToyFrameV::Core
