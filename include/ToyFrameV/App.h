#pragma once

#include "ToyFrameV/Graphics.h"
#include <string>
#include <memory>
#include <functional>
#include <chrono>

namespace ToyFrameV {

// Forward declarations
class Window;
struct InputEvent;

/**
 * @brief Application configuration
 */
struct AppConfig {
    std::string title = "ToyFrameV Application";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool resizable = true;
    bool fullscreen = false;
    GraphicsConfig graphics;  // Graphics configuration
};

/**
 * @brief Base application class
 * 
 * Users should inherit from this class and implement the virtual methods
 * to create their application.
 * 
 * The App class manages the main loop, window, and graphics context.
 * Users never need to interact with LLGL directly.
 */
class App {
public:
    App() = default;
    explicit App(const AppConfig& config);
    virtual ~App() = default;

    // Non-copyable
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    /**
     * @brief Run the application main loop
     * @return Exit code
     */
    int Run();

    /**
     * @brief Request application exit
     */
    void Quit();

    /**
     * @brief Check if application is running
     */
    bool IsRunning() const { return m_running; }

    /**
     * @brief Get application configuration
     */
    const AppConfig& GetConfig() const { return m_config; }

protected:
    /**
     * @brief Called once when application starts (after graphics is ready)
     * @return true if initialization succeeded
     */
    virtual bool OnInit() { return true; }

    /**
     * @brief Called every frame
     * @param deltaTime Time since last frame in seconds
     */
    virtual void OnUpdate(float deltaTime) {}

    /**
     * @brief Called every frame for rendering (between BeginFrame/EndFrame)
     */
    virtual void OnRender() {}

    /**
     * @brief Called once when application exits
     */
    virtual void OnShutdown() {}

    /**
     * @brief Called when window is resized
     * @param width New width
     * @param height New height
     */
    virtual void OnResize(int width, int height) {}

    /**
     * @brief Called when an input event occurs
     * @param event The input event
     */
    virtual void OnInput(const InputEvent& event) {}

    /**
     * @brief Get the graphics context
     */
    Graphics* GetGraphics() const { return m_graphics.get(); }

public:
    /**
     * @brief Execute one frame (used internally and by Emscripten)
     */
    void RunOneFrame();

protected:
    AppConfig m_config;
    bool m_running = false;
    std::unique_ptr<Graphics> m_graphics;
    std::chrono::high_resolution_clock::time_point m_lastFrameTime;
};

} // namespace ToyFrameV
