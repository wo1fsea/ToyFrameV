#include "ToyFrameV/App.h"
#include "ToyFrameV/Graphics.h"
#include "ToyFrameV/GraphicsSystem.h"
#include "ToyFrameV/IOSystem.h"
#include "ToyFrameV/Input.h"
#include "ToyFrameV/InputSystem.h"
#include "ToyFrameV/WindowSystem.h"
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
  if (!m_running) {
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
    return;
  }

  // Pre-update all systems (process events, save previous input state)
  m_systems.PreUpdateAll();

  // Check if window requested close
  auto *windowSystem = GetSystem<WindowSystem>();
  if (windowSystem && windowSystem->IsCloseRequested()) {
    m_running = false;
#ifdef __EMSCRIPTEN__
    emscripten_cancel_main_loop();
#endif
    return;
  }

  // Check if graphics is still valid
  auto *graphicsSystem = GetSystem<GraphicsSystem>();
  if (graphicsSystem && !graphicsSystem->IsValid()) {
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

    // Update all systems
    m_systems.UpdateAll(deltaTime);

    // User update
    OnUpdate(deltaTime);

    // Render phase - systems BeginFrame
    m_systems.RenderAll();

    // User render
    OnRender();

    // Post-update all systems (EndFrame/Present, reset deltas)
    m_systems.PostUpdateAll();
}

int App::Run() {
  // Register core systems in order
  // Note: Priority determines actual execution order, but registration
  // order affects initialization order for systems with same priority

  // WindowSystem - handles window creation and events (Priority: Platform = 0)
  // Creates the native window that GraphicsSystem will render to
  m_systems.AddSystem<WindowSystem>();

  // IOSystem - handles file I/O operations (Priority: Platform + 10)
  m_systems.AddSystem<IOSystem>();

  // InputSystem - handles input state per frame (Priority: Input = 100)
  m_systems.AddSystem<InputSystem>();

  // GraphicsSystem - handles rendering (Priority: Present = 1000)
  // Uses WindowSystem's window for rendering via LLGL
  m_systems.AddSystem<GraphicsSystem>(m_config.graphics);

  // Initialize all systems
  if (!m_systems.InitializeAll(this)) {
    std::cerr << "Failed to initialize systems" << std::endl;
    m_systems.ShutdownAll();
    return -1;
  }

    // User initialization
    if (!OnInit()) {
        std::cerr << "Failed to initialize application" << std::endl;
        m_systems.ShutdownAll();
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
    while (m_running) {
      RunOneFrame();
    }
#endif

    // User shutdown
    OnShutdown();

    // Shutdown all systems (reverse order)
    m_systems.ShutdownAll();

    return 0;
}

void App::Quit() {
    m_running = false;
}

Graphics *App::GetGraphics() const {
  auto *graphicsSystem = m_systems.GetSystem<GraphicsSystem>();
  return graphicsSystem ? graphicsSystem->GetGraphics() : nullptr;
}

} // namespace ToyFrameV
