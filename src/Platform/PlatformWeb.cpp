/**
 * @file PlatformWeb.cpp
 * @brief Web/Emscripten platform implementation
 * 
 * For WebGL builds, LLGL handles window/canvas management directly.
 * This file provides minimal stubs for platform-specific functionality.
 */

// Web platform uses LLGL for everything
// No additional platform-specific code needed
// LLGL's WebGL backend handles:
// - Canvas/Window management
// - Input events (via Emscripten)
// - WebGL context
