#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace ToyFrameV::Core {

enum class FutureState {
    Pending,
    Running,
    Ready,
    Cancelled,
    Failed
};

template <typename T>
class Future;

class Mutex {
  public:
    void lock() { m_mutex.lock(); }
    void unlock() { m_mutex.unlock(); }
    bool try_lock() { return m_mutex.try_lock(); }

  private:
    std::mutex m_mutex;
};

template <typename MutexT>
using LockGuard = std::lock_guard<MutexT>;

template <typename... Mutexes>
using ScopedLock = std::scoped_lock<Mutexes...>;

class SpinLock {
  public:
    void lock() {
        while (m_flag.test_and_set(std::memory_order_acquire)) {
          // Yield to avoid wasting CPU cycles in tight spin
          std::this_thread::yield();
        }
    }
    void unlock() { m_flag.clear(std::memory_order_release); }
    bool try_lock() { return !m_flag.test_and_set(std::memory_order_acquire); }

  private:
    std::atomic_flag m_flag = ATOMIC_FLAG_INIT;
};

class Semaphore {
  public:
    explicit Semaphore(uint32_t initial = 0) : m_count(initial) {}

    void Release(uint32_t n = 1);
    void Acquire();
    bool TryAcquire(uint32_t timeoutMs);

  private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    uint32_t m_count = 0;
};

namespace detail {

struct SharedStateBase {
    mutable std::mutex mutex;
    std::condition_variable cv;
    FutureState state = FutureState::Pending;
    std::exception_ptr exception;

    bool TryCancel();
    bool IsCancelled() const;
    bool IsDone() const;
    void MarkRunning();
    void MarkFailed(std::exception_ptr ex);
    void NotifyReady(FutureState finalState);
    virtual ~SharedStateBase() = default;
};

template <typename T>
struct SharedState : SharedStateBase {
    std::optional<T> value;

    template <typename U>
    void SetValue(U&& v) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            value = std::forward<U>(v);
            state = FutureState::Ready;
        }
        cv.notify_all();
    }
};

template <>
struct SharedState<void> : SharedStateBase {
    void SetValue() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            state = FutureState::Ready;
        }
        cv.notify_all();
    }
};

}  // namespace detail

/**
 * @brief Handle to an asynchronous result.
 */
template <typename T>
class Future {
  public:
    Future() = default;
    explicit Future(std::shared_ptr<detail::SharedState<T>> state) : m_state(std::move(state)) {}

    bool IsValid() const { return static_cast<bool>(m_state); }
    bool IsReady() const;
    bool IsCancelled() const;
    FutureState GetState() const;

    bool Wait(uint32_t timeoutMs = 0) const;
    bool Cancel();

    T Get();

  private:
    std::shared_ptr<detail::SharedState<T>> m_state;
};

/**
 * @brief Specialization for void
 */
template <>
class Future<void> {
  public:
    Future() = default;
    explicit Future(std::shared_ptr<detail::SharedState<void>> state) : m_state(std::move(state)) {}

    bool IsValid() const { return static_cast<bool>(m_state); }
    bool IsReady() const;
    bool IsCancelled() const;
    FutureState GetState() const;

    bool Wait(uint32_t timeoutMs = 0) const;
    bool Cancel();

    void Get();

  private:
    std::shared_ptr<detail::SharedState<void>> m_state;
};

/**
 * @brief Simple thread pool with cancellation of pending tasks.
 */
class ThreadPool {
  public:
    ThreadPool(size_t threadCount = 0, size_t maxQueueSize = 256);
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    template <typename F, typename... Args>
    auto Submit(F&& f, Args&&... args)
        -> Future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>;

    void CancelAllPending();
    void Shutdown(bool wait = true);

    size_t GetThreadCount() const { return m_threadCount; }
    size_t GetQueueSize() const;

    static ThreadPool& GetDefault(size_t threads = 0, size_t maxQueueSize = 256);

  private:
    struct Task {
        std::function<void()> fn;
        std::shared_ptr<detail::SharedStateBase> state;
    };

    void WorkerLoop();
    bool Dequeue(Task& out);

    size_t m_threadCount = 0;
    size_t m_maxQueueSize = 0;
    std::vector<std::thread> m_workers;
    std::deque<Task> m_tasks;
    mutable std::mutex m_mutex;
    std::condition_variable m_cvTasks;
    std::condition_variable m_cvSpace;
    bool m_stopping = false;
};

// === Inline / template definitions ==========================================

inline bool Semaphore::TryAcquire(uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (timeoutMs == 0) {
        m_cv.wait(lock, [&] { return m_count > 0; });
    } else {
        if (!m_cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&] { return m_count > 0; })) {
            return false;
        }
    }
    --m_count;
    return true;
}

inline void Semaphore::Acquire() { TryAcquire(0); }

inline void Semaphore::Release(uint32_t n) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_count += n;
    }
    m_cv.notify_all();
}

// SharedStateBase helpers
inline bool detail::SharedStateBase::TryCancel() {
    std::lock_guard<std::mutex> lock(mutex);
    if (state != FutureState::Pending) {
        return false;
    }
    state = FutureState::Cancelled;
    cv.notify_all();
    return true;
}

inline bool detail::SharedStateBase::IsCancelled() const {
    std::lock_guard<std::mutex> lock(mutex);
    return state == FutureState::Cancelled;
}

inline bool detail::SharedStateBase::IsDone() const {
    std::lock_guard<std::mutex> lock(mutex);
    return state == FutureState::Ready || state == FutureState::Cancelled || state == FutureState::Failed;
}

inline void detail::SharedStateBase::MarkRunning() {
    std::lock_guard<std::mutex> lock(mutex);
    if (state == FutureState::Pending) {
        state = FutureState::Running;
    }
}

inline void detail::SharedStateBase::MarkFailed(std::exception_ptr ex) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        exception = std::move(ex);
        state = FutureState::Failed;
    }
    cv.notify_all();
}

inline void detail::SharedStateBase::NotifyReady(FutureState finalState) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        state = finalState;
    }
    cv.notify_all();
}

// Future<T>
template <typename T>
bool Future<T>::IsReady() const {
    if (!m_state) return false;
    std::lock_guard<std::mutex> lock(m_state->mutex);
    return m_state->state == FutureState::Ready;
}

template <typename T>
bool Future<T>::IsCancelled() const {
    return m_state ? m_state->IsCancelled() : false;
}

template <typename T>
FutureState Future<T>::GetState() const {
    if (!m_state) return FutureState::Cancelled;
    std::lock_guard<std::mutex> lock(m_state->mutex);
    return m_state->state;
}

template <typename T>
bool Future<T>::Wait(uint32_t timeoutMs) const {
    if (!m_state) return false;
    std::unique_lock<std::mutex> lock(m_state->mutex);
    auto pred = [&] { return m_state->state == FutureState::Ready || m_state->state == FutureState::Cancelled ||
                             m_state->state == FutureState::Failed; };
    if (timeoutMs == 0) {
        m_state->cv.wait(lock, pred);
        return true;
    }
    return m_state->cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), pred);
}

template <typename T>
bool Future<T>::Cancel() {
    return m_state ? m_state->TryCancel() : false;
}

template <typename T>
T Future<T>::Get() {
    if (!m_state) {
        throw std::runtime_error("Future has no state");
    }
    Wait(0);
    std::unique_lock<std::mutex> lock(m_state->mutex);
    if (m_state->state == FutureState::Cancelled) {
        throw std::runtime_error("Future was cancelled");
    }
    if (m_state->state == FutureState::Failed && m_state->exception) {
        std::rethrow_exception(m_state->exception);
    }
    if (!m_state->value.has_value()) {
        throw std::runtime_error("Future has no value");
    }
    return std::move(*m_state->value);
}

// Future<void>
inline bool Future<void>::IsReady() const {
    if (!m_state) return false;
    std::lock_guard<std::mutex> lock(m_state->mutex);
    return m_state->state == FutureState::Ready;
}

inline bool Future<void>::IsCancelled() const {
    return m_state ? m_state->IsCancelled() : false;
}

inline FutureState Future<void>::GetState() const {
    if (!m_state) return FutureState::Cancelled;
    std::lock_guard<std::mutex> lock(m_state->mutex);
    return m_state->state;
}

inline bool Future<void>::Wait(uint32_t timeoutMs) const {
    if (!m_state) return false;
    std::unique_lock<std::mutex> lock(m_state->mutex);
    auto pred = [&] { return m_state->state == FutureState::Ready || m_state->state == FutureState::Cancelled ||
                             m_state->state == FutureState::Failed; };
    if (timeoutMs == 0) {
        m_state->cv.wait(lock, pred);
        return true;
    }
    return m_state->cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), pred);
}

inline bool Future<void>::Cancel() {
    return m_state ? m_state->TryCancel() : false;
}

inline void Future<void>::Get() {
    if (!m_state) {
        throw std::runtime_error("Future has no state");
    }
    Wait(0);
    std::unique_lock<std::mutex> lock(m_state->mutex);
    if (m_state->state == FutureState::Cancelled) {
        throw std::runtime_error("Future was cancelled");
    }
    if (m_state->state == FutureState::Failed && m_state->exception) {
        std::rethrow_exception(m_state->exception);
    }
}

// ThreadPool::Submit
template <typename F, typename... Args>
auto ThreadPool::Submit(F&& f, Args&&... args)
    -> Future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> {
    using ReturnT = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;

    auto state = std::make_shared<detail::SharedState<ReturnT>>();

#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
    // Immediate mode for single-threaded Web
    state->MarkRunning();
    try {
        if constexpr (std::is_void_v<ReturnT>) {
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            state->SetValue();
        } else {
            auto result = std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            state->SetValue(std::move(result));
        }
    } catch (...) {
        state->MarkFailed(std::current_exception());
    }
    return Future<ReturnT>(std::move(state));
#else
    Task task;
    task.state = state;
    task.fn = [state, fn = std::bind(std::forward<F>(f), std::forward<Args>(args)...)]() mutable {
        if (state->IsCancelled()) {
            state->NotifyReady(FutureState::Cancelled);
            return;
        }
        state->MarkRunning();
        try {
            if constexpr (std::is_void_v<ReturnT>) {
                fn();
                state->SetValue();
            } else {
                auto result = fn();
                state->SetValue(std::move(result));
            }
        } catch (...) {
            state->MarkFailed(std::current_exception());
        }
    };

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cvSpace.wait(lock, [&] { return m_stopping || m_tasks.size() < m_maxQueueSize; });
        if (m_stopping) {
            state->TryCancel();
            return Future<ReturnT>(std::move(state));
        }
        m_tasks.emplace_back(std::move(task));
    }
    m_cvTasks.notify_one();
    return Future<ReturnT>(std::move(state));
#endif
}

}  // namespace ToyFrameV::Core
