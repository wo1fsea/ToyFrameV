#pragma once

#include "ToyFrameV/Platform.h"
#include "ToyFrameV/Input.h"
#include <string>
#include <memory>
#include <functional>

namespace ToyFrameV {

/**
 * @brief Window configuration
 */
struct WindowConfig {
    std::string title = "ToyFrameV Window";
    int width = 1280;
    int height = 720;
    bool resizable = true;
    bool fullscreen = false;
    bool centered = true;
    int posX = 100;
    int posY = 100;
};

/**
 * @brief Window event types
 */
enum class WindowEventType {
    Close,
    Resize,
    Focus,
    LostFocus,
    Minimize,
    Maximize,
    Restore
};

/**
 * @brief Window event data
 */
struct WindowEvent {
    WindowEventType type;
    int width = 0;
    int height = 0;
};

/**
 * @brief Window event callback
 */
using WindowEventCallback = std::function<void(const WindowEvent&)>;

/**
 * @brief Abstract window class
 * 
 * Platform-specific implementations derive from this class.
 */
class Window {
public:
    virtual ~Window() = default;

    /**
     * @brief Create a window with the given configuration
     * @param config Window configuration
     * @return Pointer to created window, nullptr on failure
     */
    static std::unique_ptr<Window> Create(const WindowConfig& config = WindowConfig{});

    /**
     * @brief Process window events
     * @return true if window is still open
     */
    virtual bool ProcessEvents() = 0;

    /**
     * @brief Close the window
     */
    virtual void Close() = 0;

    /**
     * @brief Check if window is open
     */
    virtual bool IsOpen() const = 0;

    /**
     * @brief Get window width
     */
    virtual int GetWidth() const = 0;

    /**
     * @brief Get window height
     */
    virtual int GetHeight() const = 0;

    /**
     * @brief Set window title
     */
    virtual void SetTitle(const std::string& title) = 0;

    /**
     * @brief Set window size
     */
    virtual void SetSize(int width, int height) = 0;

    /**
     * @brief Set window position
     */
    virtual void SetPosition(int x, int y) = 0;

    /**
     * @brief Show/hide window
     */
    virtual void SetVisible(bool visible) = 0;

    /**
     * @brief Get native window handle (HWND, NSWindow*, etc.)
     */
    virtual void* GetNativeHandle() const = 0;

    /**
     * @brief Set event callback
     */
    void SetEventCallback(WindowEventCallback callback) {
        m_eventCallback = std::move(callback);
    }

    /**
     * @brief Set input event callback
     */
    void SetInputCallback(InputEventCallback callback) {
        m_inputCallback = std::move(callback);
    }

protected:
    Window() = default;

    void DispatchEvent(const WindowEvent& event) {
        if (m_eventCallback) {
            m_eventCallback(event);
        }
    }

    void DispatchInputEvent(const InputEvent& event) {
        if (m_inputCallback) {
            m_inputCallback(event);
        }
    }

    WindowEventCallback m_eventCallback;
    InputEventCallback m_inputCallback;
};

} // namespace ToyFrameV
