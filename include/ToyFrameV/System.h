#pragma once

/**
 * @file System.h
 * @brief System architecture for ToyFrameV framework
 * 
 * Provides a unified abstraction for subsystems (Graphics, Input, Window, etc.)
 * with lifecycle management, priority ordering, and dependency declaration.
 */

#include "ToyFrameV/Platform.h"
#include <memory>
#include <vector>
#include <string>
#include <typeinfo>
#include <functional>
#include <algorithm>

namespace ToyFrameV {

// Forward declarations
class App;
class SystemManager;

/**
 * @brief System priority levels
 * 
 * Determines the order of system updates within each frame.
 * Lower values run first.
 */
enum class SystemPriority : int {
    Platform = 0,       // Platform/Window event processing (first)
    Input = 100,        // Input state updates
    Logic = 200,        // Game logic
    Physics = 300,      // Physics simulation
    Animation = 400,    // Animation updates
    PreRender = 800,    // Pre-render preparations
    Rendering = 900,    // Rendering
    Present = 1000      // Present/Swap buffers (last)
};

/**
 * @brief Base class for all subsystems
 * 
 * All subsystems (Graphics, Input, Window, Audio, etc.) should inherit from
 * this class. The SystemManager handles lifecycle and update ordering.
 * 
 * Lifecycle:
 * 1. Initialize() - called once during startup
 * 2. PreUpdate() - called each frame before Update
 * 3. Update() - called each frame
 * 4. PostUpdate() - called each frame after Update
 * 5. Render() - called each frame for rendering
 * 6. Shutdown() - called once during cleanup (reverse order)
 */
class System {
public:
    virtual ~System() = default;

    /**
     * @brief Get the system name (for debugging/logging)
     */
    virtual const char* GetName() const = 0;

    /**
     * @brief Get the system priority (determines update order)
     * @return Priority value (lower = runs earlier)
     */
    virtual int GetPriority() const { return static_cast<int>(SystemPriority::Logic); }

    /**
     * @brief Get list of system types this system depends on
     * 
     * Override to declare dependencies. The SystemManager will ensure
     * dependencies are initialized first.
     * 
     * @return Vector of type_info pointers for dependent systems
     */
    virtual std::vector<const std::type_info*> GetDependencies() const { return {}; }

    /**
     * @brief Initialize the system
     * @param app Pointer to the application instance
     * @return true if initialization succeeded
     */
    virtual bool Initialize(App* app) { 
        m_app = app; 
        return true; 
    }

    /**
     * @brief Called at the beginning of each frame
     * 
     * Use for processing events, saving previous state, etc.
     */
    virtual void PreUpdate() {}

    /**
     * @brief Called each frame
     * @param deltaTime Time since last frame in seconds
     */
    virtual void Update(float deltaTime) {}

    /**
     * @brief Called at the end of each frame
     * 
     * Use for cleanup, resetting deltas, etc.
     */
    virtual void PostUpdate() {}

    /**
     * @brief Called during the render phase
     */
    virtual void Render() {}

    /**
     * @brief Cleanup the system
     * 
     * Called in reverse priority order during shutdown.
     */
    virtual void Shutdown() {}

    /**
     * @brief Check if system is enabled
     */
    bool IsEnabled() const { return m_enabled; }

    /**
     * @brief Enable or disable the system
     */
    void SetEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * @brief Get the owning application
     */
    App* GetApp() const { return m_app; }

protected:
    App* m_app = nullptr;
    bool m_enabled = true;
};

/**
 * @brief Manages all subsystems
 * 
 * Handles registration, initialization, update ordering, and shutdown
 * of all systems in the application.
 */
class SystemManager {
public:
    SystemManager() = default;
    ~SystemManager() = default;

    // Non-copyable
    SystemManager(const SystemManager&) = delete;
    SystemManager& operator=(const SystemManager&) = delete;

    /**
     * @brief Add a system to the manager
     * @tparam T System type (must derive from System)
     * @tparam Args Constructor argument types
     * @param args Constructor arguments
     * @return Pointer to the created system
     */
    template<typename T, typename... Args>
    T* AddSystem(Args&&... args) {
        static_assert(std::is_base_of<System, T>::value, "T must derive from System");
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        m_systems.push_back(std::move(system));
        m_sorted = false;
        return ptr;
    }

    /**
     * @brief Get a system by type
     * @tparam T System type to find
     * @return Pointer to the system, or nullptr if not found
     */
    template<typename T>
    T* GetSystem() const {
        for (const auto& sys : m_systems) {
            if (T* ptr = dynamic_cast<T*>(sys.get())) {
                return ptr;
            }
        }
        return nullptr;
    }

    /**
     * @brief Check if a system exists
     * @tparam T System type to check
     */
    template<typename T>
    bool HasSystem() const {
        return GetSystem<T>() != nullptr;
    }

    /**
     * @brief Sort systems by priority and initialize all
     * @param app Application instance
     * @return true if all systems initialized successfully
     */
    bool InitializeAll(App* app);

    /**
     * @brief Call PreUpdate on all enabled systems
     */
    void PreUpdateAll();

    /**
     * @brief Call Update on all enabled systems
     * @param deltaTime Time since last frame
     */
    void UpdateAll(float deltaTime);

    /**
     * @brief Call PostUpdate on all enabled systems
     */
    void PostUpdateAll();

    /**
     * @brief Call Render on all enabled systems
     */
    void RenderAll();

    /**
     * @brief Shutdown all systems (in reverse order)
     */
    void ShutdownAll();

    /**
     * @brief Get all registered systems
     */
    const std::vector<std::unique_ptr<System>>& GetSystems() const { return m_systems; }

    /**
     * @brief Get system count
     */
    size_t GetSystemCount() const { return m_systems.size(); }

private:
    void SortByPriority();

    std::vector<std::unique_ptr<System>> m_systems;
    bool m_sorted = false;
    bool m_initialized = false;
};

} // namespace ToyFrameV
