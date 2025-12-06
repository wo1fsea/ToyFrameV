#pragma once

/**
 * @file WindowSystem.h
 * @brief Window subsystem for ToyFrameV framework
 * 
 * Manages window creation, event processing, and provides native handle
 * for other systems (like GraphicsSystem) to use.
 */

#include "ToyFrameV/System.h"
#include "ToyFrameV/Window.h"
#include <memory>
#include <functional>

namespace ToyFrameV {

// Forward declarations
class App;

/**
 * @brief Resize callback type
 */
using ResizeCallback = std::function<void(int width, int height)>;

/**
 * @brief Window subsystem
 * 
 * Handles platform window creation and event processing.
 * Runs at Platform priority (earliest in the frame).
 * 
 * Other systems can depend on WindowSystem to get the native window handle
 * for rendering or input.
 */
class WindowSystem : public System {
public:
    WindowSystem() = default;
    explicit WindowSystem(const WindowConfig& config);
    ~WindowSystem() override = default;

    // System interface
    const char* GetName() const override { return "WindowSystem"; }
    int GetPriority() const override { return static_cast<int>(SystemPriority::Platform); }

    bool Initialize(App* app) override;
    void PreUpdate() override;
    void Shutdown() override;

    // Window access
    /**
     * @brief Get the underlying Window instance
     */
    Window* GetWindow() const { return m_window.get(); }

    /**
     * @brief Get native window handle (HWND, NSWindow*, etc.)
     */
    void* GetNativeHandle() const;

    /**
     * @brief Check if window is still open
     */
    bool IsOpen() const;

    /**
     * @brief Check if close was requested this frame
     */
    bool IsCloseRequested() const { return m_closeRequested; }

    /**
     * @brief Get window dimensions
     */
    int GetWidth() const;
    int GetHeight() const;

    /**
     * @brief Set window title
     */
    void SetTitle(const std::string& title);

    /**
     * @brief Set resize callback
     * 
     * Called when window size changes. GraphicsSystem uses this to
     * resize swap chain buffers.
     */
    void SetResizeCallback(ResizeCallback callback) {
        m_resizeCallback = std::move(callback);
    }

    /**
     * @brief Get window configuration
     */
    const WindowConfig& GetConfig() const { return m_config; }

private:
    void OnWindowEvent(const WindowEvent& event);
    void OnInputEvent(const InputEvent& event);

    WindowConfig m_config;
    std::unique_ptr<Window> m_window;
    ResizeCallback m_resizeCallback;
    bool m_closeRequested = false;
};

} // namespace ToyFrameV
