#pragma once

/**
 * @file TimerSystem.h
 * @brief Timer System for scheduling callbacks
 *
 * Provides frame-driven timers that integrate with the System lifecycle.
 * Supports one-shot (SetTimeout) and repeating (SetInterval) timers.
 *
 * Usage:
 * @code
 * auto& timer = GetSystemManager().GetSystem<TimerSystem>();
 *
 * // One-shot: fires once after 2 seconds
 * timer.SetTimeout(2.0f, []() { Log::Info("Timeout!"); });
 *
 * // Repeating: fires every 0.5 seconds
 * auto id = timer.SetInterval(0.5f, []() { Log::Info("Tick!"); });
 *
 * // Cancel later
 * timer.Cancel(id);
 * @endcode
 */

#include "ToyFrameV/System.h"
#include <functional>
#include <vector>
#include <cstdint>

namespace ToyFrameV {

/// Callback function type for timers
using TimerCallback = std::function<void()>;

/// Timer identifier for cancellation and control
using TimerId = uint32_t;

/// Invalid timer ID constant
constexpr TimerId InvalidTimerId = 0;

/**
 * @brief Timer System for scheduling callbacks
 *
 * Frame-driven timer system that supports:
 * - One-shot timers (SetTimeout)
 * - Repeating timers (SetInterval)
 * - Pause/Resume functionality
 * - Query remaining time
 */
class TimerSystem : public System {
public:
    TimerSystem() = default;
    ~TimerSystem() override = default;

    // System interface
    const char* GetName() const override { return "TimerSystem"; }
    int GetPriority() const override { return 50; } // Before InputSystem (100)

    /**
     * @brief Schedule a one-shot timer
     * @param delaySeconds Time in seconds until callback is invoked
     * @param callback Function to call when timer expires
     * @return Timer ID for cancellation/control, or InvalidTimerId on failure
     */
    TimerId SetTimeout(float delaySeconds, TimerCallback callback);

    /**
     * @brief Schedule a repeating timer
     * @param intervalSeconds Time in seconds between each invocation
     * @param callback Function to call on each interval
     * @return Timer ID for cancellation/control, or InvalidTimerId on failure
     */
    TimerId SetInterval(float intervalSeconds, TimerCallback callback);

    /**
     * @brief Cancel a timer
     * @param id Timer ID returned from SetTimeout/SetInterval
     * @return true if timer was found and cancelled
     */
    bool Cancel(TimerId id);

    /**
     * @brief Pause a timer
     *
     * Stops countdown but preserves remaining time.
     * @param id Timer ID
     * @return true if timer was found and paused
     */
    bool Pause(TimerId id);

    /**
     * @brief Resume a paused timer
     * @param id Timer ID
     * @return true if timer was found and resumed
     */
    bool Resume(TimerId id);

    /**
     * @brief Check if a timer is still active (not cancelled, not paused)
     * @param id Timer ID
     * @return true if timer is active
     */
    bool IsActive(TimerId id) const;

    /**
     * @brief Check if a timer exists (not cancelled)
     * @param id Timer ID
     * @return true if timer exists
     */
    bool Exists(TimerId id) const;

    /**
     * @brief Get remaining time for a timer
     * @param id Timer ID
     * @return Remaining seconds, or -1.0f if timer not found
     */
    float GetRemaining(TimerId id) const;

    /**
     * @brief Get number of active timers
     */
    size_t GetTimerCount() const { return timers_.size(); }

    /**
     * @brief Cancel all timers
     */
    void CancelAll();

    // System lifecycle
    void Update(float deltaTime) override;
    void Shutdown() override;

private:
    /// Internal timer state
    struct Timer {
        TimerId id = InvalidTimerId;
        float interval = 0.0f;   // 0 = one-shot, >0 = repeating
        float remaining = 0.0f;
        TimerCallback callback;
        bool paused = false;
        bool cancelled = false;
    };

    std::vector<Timer> timers_;
    TimerId nextId_ = 1;

    /// Find timer by ID (mutable)
    Timer* FindTimer(TimerId id);

    /// Find timer by ID (const)
    const Timer* FindTimer(TimerId id) const;
};

} // namespace ToyFrameV
