# ToyFrameV - Cross-Platform Graphics Framework Roadmap

## 📊 Project Overview

ToyFrameV is a lightweight cross-platform graphics framework based on LLGL, supporting Desktop and Web platforms.

## ✅ Target Platforms
- **Desktop**: Windows ✅ | macOS (Planned) | Linux (Planned)
- **Mobile**: iOS (Planned) | Android (Planned)
- **Web**: WebAssembly + WebGL ✅

## 🔧 Tech Stack
- **Build System**: CMake + Ninja (Web)
- **Graphics Backend**: [LLGL](https://github.com/LukasBanana/LLGL) (via FetchContent)
- **Web Toolchain**: Emscripten SDK

---

## 📂 Current Project Structure

```
ToyFrameV/
├── CMakeLists.txt              # Main CMake configuration
├── cmake/Platform.cmake        # Platform detection
├── scripts/build_web.ps1/sh    # Web build scripts
├── include/ToyFrameV/          # Public headers
│   ├── App.h                   # Application lifecycle
│   ├── Window.h                # Window abstraction
│   ├── Input.h                 # Input system
│   ├── KeyCodes.h              # Key code definitions
│   ├── Graphics.h              # Graphics rendering
│   └── Platform.h              # Platform abstraction
├── src/
│   ├── App.cpp                 # App implementation
│   ├── Input.cpp               # Input core implementation
│   ├── Window/WindowWindows.cpp
│   ├── Input/InputWindows.cpp
│   ├── Graphics/Graphics.cpp   # LLGL renderer wrapper
│   └── Platform/
│       ├── Windows/PlatformWindows.cpp
│       └── Web/PlatformWeb.cpp
├── samples/
│   ├── HelloApp/               # Basic application sample
│   └── HelloTriangle/          # Triangle rendering sample
├── web/template.html           # Web build template
└── docs/WebGL_Build.md         # Web build documentation
```

---

## 🗂️ Development Stages

### ✅ Stage 1: Project Initialization (Completed)
- [x] CMake project structure setup
- [x] LLGL integration via FetchContent
- [x] Platform detection macros (`cmake/Platform.cmake`)
- [x] Basic `App` class and entry points

### ✅ Stage 2: Window Creation (Completed)
- [x] Windows platform window (`WindowWindows.cpp`)
- [x] LLGL SwapChain integration
- [x] Web platform Canvas support
- [x] HelloApp sample runnable

### ✅ Stage 3: Input System (Completed)
- [x] Unified `Input` interface (`Input.h`)
- [x] Key code definitions (`KeyCodes.h`)
- [x] Windows keyboard/mouse input (`InputWindows.cpp`)
- [x] Basic input event callbacks

### ✅ Stage 4: Basic Rendering (Completed)
- [x] `Graphics` class wrapping LLGL renderer
- [x] Vertex buffer creation
- [x] Shader loading (HLSL/GLSL/SPIRV)
- [x] Render pipeline configuration
- [x] **HelloTriangle Sample**
  - [x] Windows (Direct3D 11) ✅
  - [x] Web (WebGL) ✅

### ✅ Stage 5: Web Platform Support (Completed)
- [x] Emscripten toolchain integration
- [x] `build_web.ps1` / `build_web.sh` build scripts
- [x] Web main loop (`emscripten_set_main_loop`)
- [x] HTML template (`web/template.html`)
- [x] WebGL shader compatibility

---

## 🚧 Next Stage Tasks

### 📋 Stage 6: Cross-Platform Extension
- [ ] **macOS Support**
  - [ ] Metal backend testing
  - [ ] Cocoa window creation
  - [ ] Input event handling
- [ ] **Linux Support**
  - [ ] Vulkan/OpenGL backend
  - [ ] X11/Wayland window
  - [ ] Input event handling

### 📋 Stage 7: Feature Enhancement
- [ ] **Texture System**
  - [ ] Texture loading (PNG/JPG)
  - [ ] Texture samplers
  - [ ] Textured quad rendering
- [ ] **Uniform Buffers**
  - [ ] MVP matrix passing
  - [ ] Global parameters (time, resolution, etc.)
- [ ] **Math Library**
  - [ ] Vector/matrix operations
  - [ ] Transform utility functions

### 📋 Stage 8: Mobile Support
- [ ] **Android**
  - [ ] NDK build configuration
  - [ ] EGL/OpenGL ES backend
  - [ ] Touch input
- [ ] **iOS**
  - [ ] Xcode project generation
  - [ ] Metal backend
  - [ ] Touch input

### 📋 Stage 9: Advanced Features (Long-term)
- [ ] ImGui integration
- [ ] Multi-pass rendering
- [ ] Resource manager
- [ ] Scene graph system
- [ ] Audio system

---

## ✅ Design Principles
- No `#ifdef` in user code; platform differences encapsulated in implementation layer
- Modular design: `App`, `Window`, `Input`, `Graphics` can be used independently
- LLGL as the sole graphics abstraction layer
- Minimize third-party dependencies

---

## 📝 Build Guide

### Windows (Visual Studio)
```powershell
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Web (Emscripten)
```powershell
.\scripts\build_web.ps1        # First build
.\scripts\build_web.ps1 -Clean # Clean rebuild
```

Build outputs are located in `build-web/bin/`. Use a local HTTP server to run the HTML files.

---

*Last updated: December 6, 2025*

