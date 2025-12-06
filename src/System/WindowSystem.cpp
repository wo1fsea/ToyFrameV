#include "ToyFrameV/WindowSystem.h"
#include "ToyFrameV/App.h"
#include "ToyFrameV/Input.h"
#include <iostream>

namespace ToyFrameV {

WindowSystem::WindowSystem(const WindowConfig& config)
    : m_config(config)
{
}

bool WindowSystem::Initialize(App* app) {
    System::Initialize(app);

    // If config wasn't set in constructor, try to get from AppConfig
    if (m_config.title == "ToyFrameV Window" && app) {
        const auto& appConfig = app->GetConfig();
        m_config.title = appConfig.title;
        m_config.width = appConfig.windowWidth;
        m_config.height = appConfig.windowHeight;
        m_config.resizable = appConfig.resizable;
        m_config.fullscreen = appConfig.fullscreen;
    }

#if defined(__EMSCRIPTEN__) || defined(PLATFORM_WEB)
    // On Web platform, LLGL manages the canvas directly
    // WindowSystem is still registered for API consistency but doesn't create a window
    std::cout << "WindowSystem: Web platform - LLGL manages canvas" << std::endl;
    m_closeRequested = false;
    return true;
#else
    // Create the window
    m_window = Window::Create(m_config);
    if (!m_window) {
        std::cerr << "WindowSystem: Failed to create window" << std::endl;
        return false;
    }

    // Set up event callbacks
    m_window->SetEventCallback([this](const WindowEvent& event) {
        OnWindowEvent(event);
    });

    m_window->SetInputCallback([this](const InputEvent& event) {
        OnInputEvent(event);
    });

    m_closeRequested = false;
    return true;
#endif
}

void WindowSystem::PreUpdate() {
    if (!m_window) return;

    // Reset close request flag at start of frame
    m_closeRequested = false;

    // Process platform events
    if (!m_window->ProcessEvents()) {
        m_closeRequested = true;
    }
}

void WindowSystem::Shutdown() {
    if (m_window) {
        m_window->Close();
        m_window.reset();
    }
}

void* WindowSystem::GetNativeHandle() const {
    return m_window ? m_window->GetNativeHandle() : nullptr;
}

bool WindowSystem::IsOpen() const {
    return m_window && m_window->IsOpen();
}

int WindowSystem::GetWidth() const {
    return m_window ? m_window->GetWidth() : 0;
}

int WindowSystem::GetHeight() const {
    return m_window ? m_window->GetHeight() : 0;
}

void WindowSystem::SetTitle(const std::string& title) {
    if (m_window) {
        m_window->SetTitle(title);
    }
}

void WindowSystem::OnWindowEvent(const WindowEvent& event) {
    switch (event.type) {
        case WindowEventType::Close:
            m_closeRequested = true;
            break;

        case WindowEventType::Resize:
        case WindowEventType::Maximize:
        case WindowEventType::Restore:
            // Notify resize callback (e.g., GraphicsSystem)
            if (m_resizeCallback) {
                m_resizeCallback(event.width, event.height);
            }
            // Also notify App
            if (m_app) {
                // App::OnResize will be called through the App's own mechanism
            }
            break;

        default:
            break;
    }
}

void WindowSystem::OnInputEvent(const InputEvent& event) {
    // Input state is already updated by the Window implementation
    // (WindowWindows calls Input::_SetKeyState, etc.)
    
    // Forward to App's OnInput if needed
    if (m_app) {
        // App::OnInput will be called through the App's own mechanism
    }
}

} // namespace ToyFrameV
