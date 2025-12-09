#include "ToyFrameV/System.h"
#include "ToyFrameV/Core/Log.h"
#include <algorithm>

namespace ToyFrameV {

void SystemManager::SortByPriority() {
    if (m_sorted) return;

    std::stable_sort(m_systems.begin(), m_systems.end(),
        [](const std::unique_ptr<System>& a, const std::unique_ptr<System>& b) {
            return a->GetPriority() < b->GetPriority();
        });

    m_sorted = true;
}

bool SystemManager::InitializeAll(App* app) {
    // Sort by priority before initialization
    SortByPriority();

    // TODO: Validate dependencies (ensure all dependencies exist)
    // For now, we rely on correct registration order

    for (auto& system : m_systems) {
        if (!system->Initialize(app)) {
            TOYFRAMEV_LOG_ERROR("Failed to initialize system: {}", system->GetName());
            // Shutdown already initialized systems in reverse order
            for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
                if (it->get() == system.get()) break;
                (*it)->Shutdown();
            }
            return false;
        }
    }

    m_initialized = true;
    return true;
}

void SystemManager::PreUpdateAll() {
    for (auto& system : m_systems) {
        if (system->IsEnabled()) {
            system->PreUpdate();
        }
    }
}

void SystemManager::UpdateAll(float deltaTime) {
    for (auto& system : m_systems) {
        if (system->IsEnabled()) {
            system->Update(deltaTime);
        }
    }
}

void SystemManager::PostUpdateAll() {
    for (auto& system : m_systems) {
        if (system->IsEnabled()) {
            system->PostUpdate();
        }
    }
}

void SystemManager::RenderAll() {
    for (auto& system : m_systems) {
        if (system->IsEnabled()) {
            system->Render();
        }
    }
}

void SystemManager::ShutdownAll() {
    if (!m_initialized) return;

    // Shutdown in reverse priority order
    for (auto it = m_systems.rbegin(); it != m_systems.rend(); ++it) {
        (*it)->Shutdown();
    }

    m_systems.clear();
    m_initialized = false;
}

} // namespace ToyFrameV
