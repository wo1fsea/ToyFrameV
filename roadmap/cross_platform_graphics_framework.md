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
│   ├── Graphics.h              # Graphics aggregate header
│   ├── Graphics/               # Graphics submodules
│   │   ├── Types.h             # Color, Format, VertexLayout
│   │   ├── Buffer.h            # Buffer class
│   │   ├── Shader.h            # Shader class
│   │   ├── Pipeline.h          # Pipeline class
│   │   ├── RenderTexture.h     # Offscreen render target
│   │   └── Context.h           # Graphics main class
│   ├── Platform.h              # Platform abstraction
│   ├── System.h                # System base class
│   ├── WindowSystem.h          # Window subsystem
│   ├── GraphicsSystem.h        # Graphics subsystem
│   ├── InputSystem.h           # Input subsystem
│   ├── IOSystem.h              # I/O subsystem (file/network)
│   ├── TimerSystem.h           # Timer subsystem
│   └── Core/                   # Core utilities
│       ├── Log.h               # Logging API
│       └── Threading.h         # ThreadPool/Future and sync primitives
├── src/
│   ├── App.cpp                 # App implementation
│   ├── Input.cpp               # Input core implementation
│   ├── Core/Threading.cpp      # ThreadPool/Future implementation
│   ├── Core/Log.cpp            # Logging implementation
│   ├── Window/WindowWindows.cpp
│   ├── Input/InputWindows.cpp
│   ├── Graphics/               # Graphics implementations
│   │   ├── Graphics.cpp        # LLGL renderer wrapper
│   │   └── RenderTexture.cpp   # Offscreen render target
│   ├── System/                 # System implementations
│   │   ├── SystemManager.cpp   # System lifecycle management
│   │   ├── WindowSystem.cpp
│   │   ├── GraphicsSystem.cpp
│   │   ├── InputSystem.cpp
│   │   ├── IOSystem.cpp        # I/O system implementation
│   │   └── TimerSystem.cpp     # Timer system implementation
│   └── Platform/
│       ├── Windows/PlatformWindows.cpp
│       └── Web/PlatformWeb.cpp
├── samples/
│   ├── HelloApp/               # Basic application sample
│   ├── HelloTriangle/          # Triangle rendering sample
│   ├── HelloIO/                # I/O system sample
│   ├── HelloThreadLog/         # ThreadPool + Log sample
│   ├── HelloTimer/             # Timer system sample
│   └── HelloRenderTexture/     # Offscreen rendering sample
├── third_party/fmt/core.h      # Minimal header-only fmt-style formatter
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

### ✅ Stage 6: System Architecture (Completed)
- [x] **System Base Class** (`System.h`)
  - [x] Unified lifecycle: `Initialize`, `PreUpdate`, `Update`, `PostUpdate`, `Render`, `Shutdown`
  - [x] Priority-based ordering (`SystemPriority` enum)
  - [x] Dependency declaration via `GetDependencies()`
- [x] **SystemManager** (`SystemManager.cpp`)
  - [x] System registration (`AddSystem<T>()`)
  - [x] System retrieval (`GetSystem<T>()`)
  - [x] Batch lifecycle calls with priority sorting
  - [x] Reverse-order shutdown
- [x] **Core Systems Implemented**
  - [x] `WindowSystem` - Platform window management (Priority: 0)
  - [x] `InputSystem` - Input state per-frame updates (Priority: 100)
  - [x] `GraphicsSystem` - Rendering context and frame management (Priority: 1000)
- [x] **IOSystem** (`IOSystem.h`, `IOSystem.cpp`)
  - [x] Zero-copy `IOBuffer` with move semantics
  - [x] Path schemes (`assets://`, `documents://`, `http://`, etc.)
  - [x] Sync/Async file I/O API
  - [x] Platform-specific directories
- [x] **App Refactored** to use `SystemManager`

### ✅ Stage 7: Core Utilities (Completed)
Low-level utilities used by Systems and user code. ThreadPool uses standard threads; Web without pthreads falls back to immediate execution; Log uses header-only fmt-style formatter.

#### 7.1 Threading Module
```
include/ToyFrameV/Core/Threading.h
src/Core/Threading.cpp
```
- [x] **ThreadPool**
  - [x] Worker thread pool with configurable size (default = hardware concurrency)
  - [x] `Submit(task)` returning `Future<T>`
  - [x] `GetDefault()` singleton access (parameters only used on first call)
  - [x] Graceful shutdown with pending-task cancellation option
- [x] **Task/Future**
  - [x] `Task<T>` - callable wrapper
  - [x] `Future<T>` - result with `Wait()`, `Get()`, `IsReady()`, `Cancel()`
  - [x] Exception propagation
- [x] **Synchronization Primitives**
  - [x] `Mutex`, `LockGuard`, `ScopedLock`
  - [x] `SpinLock` - with `yield()` to avoid CPU waste
  - [x] `Semaphore` - counting semaphore
- [x] **Platform Implementation**
  - [x] Windows: `std::thread`-based workers
  - [x] Web: Single-threaded fallback when pthreads unavailable

#### 7.2 Log Module
```
include/ToyFrameV/Core/Log.h
src/Core/Log.cpp
```
- [x] **Log Levels**
  - [x] `Trace`, `Debug`, `Info`, `Warning`, `Error`, `Fatal`
  - [x] Runtime level filtering
  - [x] Compile-time level stripping (Release)
- [x] **Log Interface** (use macros for correct source location)
  ```cpp
  TOYFRAMEV_LOG_INFO("Player {} joined", playerId);
  TOYFRAMEV_LOG_ERROR("Failed to load: {}", filename);
  ```
- [x] **Log Sinks** (outputs)
  - [x] Console sink (stdout with colors, synchronous)
  - [x] File sink (rotating files, async worker, `flushEachMessage` option)
  - [x] Custom sink interface for extensions
- [x] **Features**
  - [x] Source location (file, line, function)
  - [x] Timestamp + thread id formatting
  - [x] Category/tag filtering
  - [x] Thread-safe dispatch, async file buffering
  - [x] Always flush file on shutdown
- [x] **Platform Support**
  - [x] Windows: Console colors via Win32 API (with error handling)
  - [x] Web: `console.log()` / `console.error()` via Emscripten

#### 7.3 fmt-style Formatter
```
third_party/fmt/core.h
```
- [x] **Minimal header-only implementation**
  - [x] `fmt::format()` with `{}` placeholders
  - [x] Proper `{{` and `}}` escape sequence handling
  - [x] Type-safe argument formatting via `std::ostringstream`

- **Samples**
  - [x] `HelloThreadLog` sample with comprehensive edge case tests:
    - All log levels
    - Format string edge cases (empty, escapes, long strings, special chars)
    - Level filtering test
    - Category filtering test
    - Rapid logging stress test
    - ThreadPool task execution

---

## 🚧 Next Stage Tasks

### 📋 Stage 8: Debug Systems (TODO)
Debug-only Systems (stripped in Release builds).

#### 8.1 ConsoleSystem
```
include/ToyFrameV/ConsoleSystem.h
src/System/ConsoleSystem.cpp
```
- [ ] **Console UI**
  - [ ] Toggle with `` ` `` (backtick) key
  - [ ] Semi-transparent overlay rendering
  - [ ] Text input field with cursor
  - [ ] Scrollable output history
- [ ] **Command System**
  - [ ] `RegisterCommand(name, callback, help)`
  - [ ] Command auto-completion (Tab)
  - [ ] Command history (Up/Down arrows)
  - [ ] Argument parsing
- [ ] **Built-in Commands**
  - [ ] `help` - list all commands
  - [ ] `clear` - clear output
  - [ ] `quit` - exit application
  - [ ] `systems` - list registered systems
  - [ ] `fps` - toggle FPS display
  - [ ] `set <cvar> <value>` - modify CVars
- [ ] **Log Integration**
  - [ ] Subscribe to Log output
  - [ ] Color-coded log levels
  - [ ] Filter by log level/category
- [ ] **CVar System** (Console Variables)
  ```cpp
  CVar<float> g_gravity("physics.gravity", 9.8f, "Gravity acceleration");
  // In console: set physics.gravity 20.0
  ```
- [ ] **Text Rendering**
  - [ ] Simple bitmap font or LLGL text rendering
  - [ ] Fixed-width font for alignment
- [ ] **Priority**: `SystemPriority::DebugUI` (850)
  - [ ] Update: Process input when console open
  - [ ] Render: Draw after scene, before present

#### 8.2 Debug Macros
```cpp
// Compile-time stripping
#ifdef TOYFRAMEV_DEBUG
    #define TOYFRAMEV_ASSERT(cond, msg) ...
    #define TOYFRAMEV_LOG_DEBUG(...) Log::Debug(__VA_ARGS__)
#else
    #define TOYFRAMEV_ASSERT(cond, msg) ((void)0)
    #define TOYFRAMEV_LOG_DEBUG(...) ((void)0)
#endif
```

### 📋 Stage 9: System Architecture Enhancement (In Progress)
- [x] **TimerSystem** (`TimerSystem.h`, `TimerSystem.cpp`)
  - [x] One-shot timer: `SetTimeout(delay, callback)` returns `TimerId`
  - [x] Repeating timer: `SetInterval(interval, callback)` returns `TimerId`
  - [x] Timer control: `Cancel(id)`, `Pause(id)`, `Resume(id)`
  - [x] Query: `IsActive(id)`, `GetRemaining(id)`, `CancelAll()`
  - [x] Frame-driven updates in `Update(deltaTime)`
  - [x] Automatic cleanup of completed one-shot timers
  - [x] Priority: 50 (before InputSystem)
  - [x] `HelloTimer` sample demonstrating all features
- [ ] **Event Bus System**
  - [ ] `EventBus` class with `Publish<T>()` / `Subscribe<T>()`
  - [ ] Decouple system-to-system communication
  - [ ] Window resize events via EventBus
  - [ ] Input events via EventBus
- [ ] **Configuration-Driven Registration**
  - [ ] JSON/YAML config file support
  - [ ] Dynamic system loading from config
  - [ ] System parameter configuration

### 📋 Stage 10: Cross-Platform Extension
- [ ] **macOS Support**
  - [ ] Metal backend testing
  - [ ] Cocoa window creation
  - [ ] Input event handling
- [ ] **Linux Support**
  - [ ] Vulkan/OpenGL backend
  - [ ] X11/Wayland window
  - [ ] Input event handling

### 📋 Stage 11: Feature Enhancement (In Progress)
- [x] **RenderTexture System** (`Graphics/RenderTexture.h`, `RenderTexture.cpp`)
  - [x] Offscreen render target creation
  - [x] `SetRenderTarget()` / `GetRenderTarget()` API
  - [x] Synchronous pixel readback (`ReadPixels()`)
  - [x] Async readback API (`ReadPixelsAsync()`) for WebGL
  - [x] BMP file export (`SaveToBMP()`)
  - [x] `HelloRenderTexture` sample
- [x] **Graphics Module Refactor**
  - [x] Split `Graphics.h` into `Graphics/` subdirectory
  - [x] `Types.h`, `Buffer.h`, `Shader.h`, `Pipeline.h`, `RenderTexture.h`, `Context.h`
  - [x] Aggregate `Graphics.h` includes all submodules
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

### 📋 Stage 12: Mobile Support
- [ ] **Android**
  - [ ] NDK build configuration
  - [ ] EGL/OpenGL ES backend
  - [ ] Touch input
- [ ] **iOS**
  - [ ] Xcode project generation
  - [ ] Metal backend
  - [ ] Touch input

### 📋 Stage 13: Advanced Features (Long-term)
- [ ] ImGui integration (alternative to ConsoleSystem)
- [ ] Multi-pass rendering
- [ ] Resource manager
- [ ] Scene graph system
- [ ] Audio system (as AudioSystem)
- [ ] Physics system (as PhysicsSystem)

---

## 🏗️ System Architecture

### System Priority Order
```
┌─────────────────────────────────────────────────────────────┐
│  Priority 0    │ WindowSystem   │ Platform events           │
├─────────────────────────────────────────────────────────────┤
│  Priority 10   │ IOSystem       │ File/Network I/O          │
├─────────────────────────────────────────────────────────────┤
│  Priority 50   │ TimerSystem    │ Timed callbacks           │
├─────────────────────────────────────────────────────────────┤
│  Priority 100  │ InputSystem    │ Input state updates       │
├─────────────────────────────────────────────────────────────┤
│  Priority 200  │ (User Logic)   │ Game logic systems        │
├─────────────────────────────────────────────────────────────┤
│  Priority 300  │ (Physics)      │ Physics simulation        │
├─────────────────────────────────────────────────────────────┤
│  Priority 850  │ ConsoleSystem  │ Debug UI overlay          │
├─────────────────────────────────────────────────────────────┤
│  Priority 900  │ (PreRender)    │ Render preparations       │
├─────────────────────────────────────────────────────────────┤
│  Priority 1000 │ GraphicsSystem │ Frame present/swap        │
└─────────────────────────────────────────────────────────────┘
```

### Core Utilities vs Systems
```
┌─────────────────────────────────────────────────────────────┐
│  Core Utilities (No frame updates, stateless services)      │
│  ├── Threading  │ ThreadPool, Task, Future, Mutex          │
│  ├── Log        │ Logging with levels, sinks, formatting   │
│  └── (Future)   │ Memory allocators, Math library          │
├─────────────────────────────────────────────────────────────┤
│  Systems (Frame-driven, stateful, lifecycle-managed)        │
│  ├── WindowSystem, InputSystem, GraphicsSystem, IOSystem   │
│  └── ConsoleSystem, AudioSystem, PhysicsSystem (future)    │
└─────────────────────────────────────────────────────────────┘
```

### Frame Execution Order
```
PreUpdate (ascending priority)
    ├── InputSystem: Save previous frame state
    ├── WindowSystem: Process platform events
    └── GraphicsSystem: Process LLGL events
         ↓
Update (ascending priority)
    └── All systems + OnUpdate()
         ↓
Render
    ├── GraphicsSystem: BeginFrame()
    └── OnRender()
         ↓
PostUpdate (ascending priority)
    ├── GraphicsSystem: EndFrame/Present
    └── InputSystem: Reset scroll delta
```

### Adding Custom Systems
```cpp
class MyGameSystem : public ToyFrameV::System {
public:
    const char* GetName() const override { return "MyGameSystem"; }
    int GetPriority() const override { return 200; } // Logic priority
    
    bool Initialize(App* app) override { /* ... */ return true; }
    void Update(float deltaTime) override { /* game logic */ }
};

// In App subclass:
bool OnInit() override {
    GetSystemManager().AddSystem<MyGameSystem>();
    return true;
}
```

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

*Last updated: December 9, 2025*
