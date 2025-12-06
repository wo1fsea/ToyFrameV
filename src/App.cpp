#include "ToyFrameV/App.h"
#include "ToyFrameV/Graphics.h"
#include "ToyFrameV/Input.h"
#include <chrono>
#include <iostream>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace ToyFrameV {

// For Emscripten main loop callback
static App* g_currentApp = nullptr;

App::App(const AppConfig& config)
    : m_config(config)
{
}

#ifdef __EMSCRIPTEN__
// Emscripten main loop callback
static void EmscriptenMainLoop() {
    if (g_currentApp) {
        g_currentApp->RunOneFrame();
    }
}
#endif

void App::RunOneFrame() {
    if (!m_running || !m_graphics->ProcessEvents()) {
        m_running = false;
#ifdef __EMSCRIPTEN__
        emscripten_cancel_main_loop();
#endif
        return;
    }

    // Calculate delta time
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();
    m_lastFrameTime = currentTime;

    // User update
    OnUpdate(deltaTime);

    // Begin frame
    m_graphics->BeginFrame();

    // User render
    OnRender();

    // End frame (present)
    m_graphics->EndFrame();
}

int App::Run() {
    // Create graphics context (this also creates the window via LLGL)
    m_graphics = Graphics::Create(nullptr, m_config.graphics);
    if (!m_graphics) {
        std::cerr << "Failed to create graphics context" << std::endl;
        return -1;
    }

    // User initialization
    if (!OnInit()) {
        std::cerr << "Failed to initialize application" << std::endl;
        m_graphics.reset();
        return -1;
    }

    m_running = true;
    m_lastFrameTime = std::chrono::high_resolution_clock::now();

#ifdef __EMSCRIPTEN__
    // Emscripten: Use browser's requestAnimationFrame
    g_currentApp = this;
    emscripten_set_main_loop(EmscriptenMainLoop, 0, 1);
    
    // After loop ends (won't reach here normally in Emscripten)
    g_currentApp = nullptr;
#else
    // Desktop: Traditional while loop
    while (m_running && m_graphics->ProcessEvents()) {
        RunOneFrame();
    }
#endif

    // User shutdown
    OnShutdown();

    // Cleanup
    m_graphics.reset();

    return 0;
}

void App::Quit() {
    m_running = false;
}

} // namespace ToyFrameV
