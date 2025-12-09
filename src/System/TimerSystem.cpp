/**
 * @file TimerSystem.cpp
 * @brief Timer System implementation
 */

#include "ToyFrameV/TimerSystem.h"
#include <algorithm>

namespace ToyFrameV {

TimerId TimerSystem::SetTimeout(float delaySeconds, TimerCallback callback) {
    if (!callback || delaySeconds < 0.0f) {
        return InvalidTimerId;
    }

    Timer timer;
    timer.id = nextId_++;
    timer.interval = 0.0f;  // One-shot
    timer.remaining = delaySeconds;
    timer.callback = std::move(callback);

    timers_.push_back(std::move(timer));
    return timers_.back().id;
}

TimerId TimerSystem::SetInterval(float intervalSeconds, TimerCallback callback) {
    if (!callback || intervalSeconds <= 0.0f) {
        return InvalidTimerId;
    }

    Timer timer;
    timer.id = nextId_++;
    timer.interval = intervalSeconds;  // Repeating
    timer.remaining = intervalSeconds;
    timer.callback = std::move(callback);

    timers_.push_back(std::move(timer));
    return timers_.back().id;
}

bool TimerSystem::Cancel(TimerId id) {
    if (auto* timer = FindTimer(id)) {
        timer->cancelled = true;
        return true;
    }
    return false;
}

bool TimerSystem::Pause(TimerId id) {
    if (auto* timer = FindTimer(id)) {
        timer->paused = true;
        return true;
    }
    return false;
}

bool TimerSystem::Resume(TimerId id) {
    if (auto* timer = FindTimer(id)) {
        timer->paused = false;
        return true;
    }
    return false;
}

bool TimerSystem::IsActive(TimerId id) const {
    const auto* timer = FindTimer(id);
    return timer && !timer->cancelled && !timer->paused;
}

bool TimerSystem::Exists(TimerId id) const {
    const auto* timer = FindTimer(id);
    return timer && !timer->cancelled;
}

float TimerSystem::GetRemaining(TimerId id) const {
    const auto* timer = FindTimer(id);
    if (timer && !timer->cancelled) {
        return timer->remaining;
    }
    return -1.0f;
}

void TimerSystem::CancelAll() {
    for (auto& timer : timers_) {
        timer.cancelled = true;
    }
}

void TimerSystem::Update(float deltaTime) {
    // Update all active timers
    for (auto& timer : timers_) {
        if (timer.cancelled || timer.paused) {
            continue;
        }

        timer.remaining -= deltaTime;

        if (timer.remaining <= 0.0f) {
            // Invoke callback
            if (timer.callback) {
                timer.callback();
            }

            if (timer.interval > 0.0f) {
                // Repeating: reset with interval
                // Handle case where deltaTime > interval (catch up without multiple calls)
                while (timer.remaining <= 0.0f) {
                    timer.remaining += timer.interval;
                }
            } else {
                // One-shot: mark for removal
                timer.cancelled = true;
            }
        }
    }

    // Remove cancelled timers
    timers_.erase(
        std::remove_if(timers_.begin(), timers_.end(),
            [](const Timer& t) { return t.cancelled; }),
        timers_.end()
    );
}

void TimerSystem::Shutdown() {
    CancelAll();
    timers_.clear();
}

TimerSystem::Timer* TimerSystem::FindTimer(TimerId id) {
    for (auto& timer : timers_) {
        if (timer.id == id && !timer.cancelled) {
            return &timer;
        }
    }
    return nullptr;
}

const TimerSystem::Timer* TimerSystem::FindTimer(TimerId id) const {
    for (const auto& timer : timers_) {
        if (timer.id == id && !timer.cancelled) {
            return &timer;
        }
    }
    return nullptr;
}

} // namespace ToyFrameV
