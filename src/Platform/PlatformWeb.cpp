/**
 * @file PlatformWeb.cpp
 * @brief Web/Emscripten platform implementation
 * 
 * For WebGL builds, LLGL handles window/canvas management directly.
 * This file provides minimal stubs for platform-specific functionality.
 */

#include "ToyFrameV/Window.h"

namespace ToyFrameV {

// ============================================================================
// Window stub for Web platform
// On Web, LLGL manages the canvas directly, so Window::Create returns nullptr.
// WindowSystem and GraphicsSystem handle this gracefully.
// ============================================================================

std::unique_ptr<Window> Window::Create(const WindowConfig &config) {
  // On Web platform, we don't create a separate window
  // LLGL manages the HTML5 canvas directly
  return nullptr;
}

} // namespace ToyFrameV
