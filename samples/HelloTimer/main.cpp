//==============================================================================
// ToyFrameV - HelloTimer Sample
// Demonstrates TimerSystem usage: SetTimeout, SetInterval, Pause, Resume, Cancel
//==============================================================================

#include <ToyFrameV.h>
#include <iostream>

using namespace ToyFrameV;
using namespace ToyFrameV::Core;

class HelloTimerApp : public App {
public:
    HelloTimerApp() {
        m_config.title = "Hello TimerSystem";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
    }

protected:
    bool OnInit() override {
        std::cout << "========================================" << std::endl;
        std::cout << "    HelloTimer - TimerSystem Demo" << std::endl;
        std::cout << "========================================" << std::endl;

        // Get TimerSystem reference
        auto* timerSystem = GetSystem<TimerSystem>();
        if (!timerSystem) {
            std::cerr << "TimerSystem not available!" << std::endl;
            return false;
        }

        // Example 1: One-shot timer (SetTimeout)
        TOYFRAMEV_LOG_INFO("[1] SetTimeout - One-shot timer after 2 seconds");
        m_timeoutId = timerSystem->SetTimeout(2.0, []() {
            TOYFRAMEV_LOG_INFO("[Timeout] 2 seconds elapsed! This fires only once.");
        });

        // Example 2: Repeating timer (SetInterval)
        TOYFRAMEV_LOG_INFO("[2] SetInterval - Repeating every 1 second");
        m_intervalId = timerSystem->SetInterval(1.0, [this]() {
            m_intervalCount++;
            TOYFRAMEV_LOG_INFO("[Interval] Tick #{} - Fires every second", m_intervalCount);
            
            // Auto-stop after 5 ticks
            if (m_intervalCount >= 5) {
                TOYFRAMEV_LOG_INFO("Interval reached 5 ticks, will be cancelled soon");
            }
        });

        // Example 3: Timer that will be cancelled
        TOYFRAMEV_LOG_INFO("[3] Creating a timer that will be cancelled after 1.5 seconds");
        m_cancelledId = timerSystem->SetTimeout(3.0, []() {
            TOYFRAMEV_LOG_ERROR("[Should NOT see this] This timer was supposed to be cancelled!");
        });

        // Example 4: Timer to cancel the above timer
        timerSystem->SetTimeout(1.5, [this]() {
            TOYFRAMEV_LOG_INFO("[Canceller] Cancelling the 3-second timer now!");
            auto* timer = GetSystem<TimerSystem>();
            timer->Cancel(m_cancelledId);
            TOYFRAMEV_LOG_INFO("Timer cancelled successfully");
        });

        // Example 5: Pause/Resume demonstration
        TOYFRAMEV_LOG_INFO("[5] Creating a pausable timer (will pause at 2.5s, resume at 4s)");
        m_pausableId = timerSystem->SetTimeout(3.0, []() {
            TOYFRAMEV_LOG_INFO("[Pausable] This timer was paused and resumed!");
        });

        // Pause at 2.5 seconds
        timerSystem->SetTimeout(2.5, [this]() {
            TOYFRAMEV_LOG_INFO("[Control] Pausing the pausable timer...");
            auto* timer = GetSystem<TimerSystem>();
            timer->Pause(m_pausableId);
            
            double remaining = timer->GetRemaining(m_pausableId);
            TOYFRAMEV_LOG_INFO("[Control] Timer paused with {:.2f} seconds remaining", remaining);
        });

        // Resume at 4 seconds
        timerSystem->SetTimeout(4.0, [this]() {
            TOYFRAMEV_LOG_INFO("[Control] Resuming the pausable timer...");
            auto* timer = GetSystem<TimerSystem>();
            timer->Resume(m_pausableId);
        });

        // Example 6: Check timer status
        TOYFRAMEV_LOG_INFO("[6] Timer status checking");
        timerSystem->SetTimeout(0.5, [this]() {
            auto* timer = GetSystem<TimerSystem>();
            TOYFRAMEV_LOG_INFO("[Status] Timeout timer active: {}", timer->IsActive(m_timeoutId));
            TOYFRAMEV_LOG_INFO("[Status] Interval timer active: {}", timer->IsActive(m_intervalId));
            TOYFRAMEV_LOG_INFO("[Status] Timeout remaining: {:.2f}s", timer->GetRemaining(m_timeoutId));
        });

        // Example 7: Final cleanup timer
        timerSystem->SetTimeout(8.0, [this]() {
            TOYFRAMEV_LOG_INFO("\n[Cleanup] Stopping interval timer and ending demo...");
            auto* timer = GetSystem<TimerSystem>();
            timer->Cancel(m_intervalId);
            TOYFRAMEV_LOG_INFO("=== HelloTimer Demo Complete ===");
            Quit();
        });

        TOYFRAMEV_LOG_INFO("");
        TOYFRAMEV_LOG_INFO("Starting main loop - watch the timed events!");
        TOYFRAMEV_LOG_INFO("----------------------------------------");
        
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // TimerSystem is updated automatically by SystemManager
    }

    void OnShutdown() override {
        std::cout << "HelloTimer shutdown!" << std::endl;
    }

private:
    TimerId m_timeoutId = 0;
    TimerId m_intervalId = 0;
    TimerId m_cancelledId = 0;
    TimerId m_pausableId = 0;
    int m_intervalCount = 0;
};

// Entry point
TOYFRAMEV_MAIN(HelloTimerApp)
