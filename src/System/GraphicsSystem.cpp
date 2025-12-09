#include "ToyFrameV/GraphicsSystem.h"
#include "ToyFrameV/WindowSystem.h"
#include "ToyFrameV/App.h"
#include "ToyFrameV/Core/Log.h"

namespace ToyFrameV {

GraphicsSystem::GraphicsSystem(const GraphicsConfig& config)
    : m_config(config)
{
}

std::vector<const std::type_info*> GraphicsSystem::GetDependencies() const {
    // GraphicsSystem depends on WindowSystem
    return { &typeid(WindowSystem) };
}

bool GraphicsSystem::Initialize(App* app) {
    System::Initialize(app);

    // If config wasn't set in constructor, try to get from AppConfig
    if (app) {
        const auto& appConfig = app->GetConfig();
        if (m_config.backend == GraphicsBackend::Auto) {
            m_config = appConfig.graphics;
        }
    }

    // Get WindowSystem and its window
    Window* window = nullptr;
    if (app) {
        auto* windowSystem = app->GetSystem<WindowSystem>();
        if (windowSystem) {
            window = windowSystem->GetWindow();
            
            // Register for resize events
            windowSystem->SetResizeCallback([this](int width, int height) {
                OnResize(width, height);
            });
        }
    }

    // Create graphics context with external window (if available)
    // If window is nullptr, Graphics will create its own LLGL window
    m_graphics = Graphics::Create(window, m_config);
    if (!m_graphics) {
        TOYFRAMEV_LOG_ERROR("GraphicsSystem: Failed to create graphics context");
        return false;
    }

    TOYFRAMEV_LOG_INFO("GraphicsSystem: Initialized with {}", 
              (window ? "external ToyFrameV window" : "LLGL-managed window"));

    return true;
}

void GraphicsSystem::PreUpdate() {
    if (!m_graphics) return;

    // Process events - behavior depends on window ownership
    // When using external window: just checks swap chain validity
    // When LLGL owns window: processes LLGL window events
    if (!m_graphics->ProcessEvents()) {
        // Signal app to quit
        if (m_app) {
            m_app->Quit();
        }
    }
}

void GraphicsSystem::Render() {
    if (!m_graphics) return;

    // Begin frame (starts render pass)
    m_graphics->BeginFrame();
    m_frameStarted = true;
}

void GraphicsSystem::PostUpdate() {
    if (!m_graphics || !m_frameStarted) return;

    // End frame (presents)
    m_graphics->EndFrame();
    m_frameStarted = false;
}

void GraphicsSystem::Shutdown() {
    m_graphics.reset();
}

void GraphicsSystem::OnResize(int width, int height) {
    if (m_graphics && width > 0 && height > 0) {
        m_graphics->OnResize(width, height);
    }
}

void GraphicsSystem::Clear(const Color& color) {
    if (m_graphics) {
        m_graphics->Clear(color);
    }
}

const std::string& GraphicsSystem::GetBackendName() const {
    static const std::string empty;
    return m_graphics ? m_graphics->GetBackendName() : empty;
}

const std::string& GraphicsSystem::GetDeviceName() const {
    static const std::string empty;
    return m_graphics ? m_graphics->GetDeviceName() : empty;
}

} // namespace ToyFrameV
