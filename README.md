# ToyFrameV

A cross-platform C++ game framework with modern graphics, input, and I/O systems.

## Features

- **Cross-Platform**: Supports Windows, Linux, macOS, Web (via Emscripten), iOS, and Android
- **Modern Graphics**: OpenGL, DirectX 11, Metal, and WebGL backends via LLGL
- **Input System**: Unified keyboard, mouse, and touch input handling
- **I/O System**: Async file loading with platform-specific path management
- **Threading**: Built-in thread pool and logging system with {fmt} formatting
- **Lightweight**: Minimal dependencies, easy to integrate

## Platform Status

| Platform | Status | Graphics Backend |
|----------|--------|------------------|
| Linux    | ✅ Working | OpenGL |
| Windows  | ✅ Working | DirectX 11 / OpenGL |
| Web      | ✅ Working | WebGL |
| macOS    | 🚧 In Progress | Metal / OpenGL |
| iOS      | 🚧 Planned | Metal |
| Android  | 🚧 Planned | OpenGL ES |

## Building

### Prerequisites

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git
sudo apt-get install -y libgl1-mesa-dev libglu1-mesa-dev
sudo apt-get install -y libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

#### Windows
- Visual Studio 2019 or later
- CMake 3.20 or later

#### macOS
```bash
brew install cmake
```

### Compile

```bash
# Clone the repository
git clone https://github.com/wo1fsea/ToyFrameV.git
cd ToyFrameV

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
cmake --build . -j$(nproc)
```

The compiled examples will be in `build/bin/`:
- `HelloApp` - Basic framework demo
- `HelloTriangle` - Simple triangle rendering
- `HelloIO` - I/O system demonstration
- `HelloThreadLog` - Threading and logging demo

## Running Examples

### On Linux with GUI
Simply run the executables:
```bash
./build/bin/HelloApp
```

### On Linux without Display (e.g., Docker, CI/CD)

Use Xvfb (virtual framebuffer):

```bash
# Install Xvfb
sudo apt-get install -y xvfb

# Method 1: Use the provided script
./scripts/linux/run_with_xvfb.sh ./build/bin/HelloApp

# Method 2: Manual
Xvfb :99 -screen 0 1280x720x24 &
export DISPLAY=:99
./build/bin/HelloApp
```

See [docs/Xvfb_Usage.md](docs/Xvfb_Usage.md) for more details.

## Dependencies

### External Libraries

- **[LLGL](https://github.com/LukasBanana/LLGL)** - Low Level Graphics Library (auto-downloaded via CMake FetchContent)
  - Provides cross-platform graphics abstraction for OpenGL, DirectX, Metal, etc.
  
- **{fmt}** - Modern formatting library (header-only, included in `third_party/fmt/`)
  - Used for string formatting in the logging system
  - Only `fmt::format` with `{}` placeholders is used

### System Dependencies (Linux)

- OpenGL development libraries (`libgl1-mesa-dev`, `libglu1-mesa-dev`)
- X11 development libraries (`libx11-dev`, `libxrandr-dev`, `libxinerama-dev`, `libxcursor-dev`, `libxi-dev`)
- Xvfb for headless environments (optional)

## Project Structure

```
ToyFrameV/
├── include/          # Public headers
│   └── ToyFrameV/    # Framework headers
├── src/              # Implementation
│   ├── Core/         # Core utilities (logging, threading)
│   ├── Graphics/     # Graphics abstraction
│   ├── Platform/     # Platform-specific code
│   └── System/       # System managers
├── samples/          # Example applications
├── scripts/          # Build and utility scripts
└── docs/             # Documentation
```

## Examples

### Basic Application

```cpp
#include <ToyFrameV.h>

class MyApp : public ToyFrameV::App {
protected:
    bool OnInit() override {
        // Initialize your app
        return true;
    }
    
    void OnUpdate(float deltaTime) override {
        // Update logic
    }
    
    void OnRender() override {
        // Render your scene
        GetGraphics()->Clear({0.2f, 0.3f, 0.4f});
    }
};

int main() {
    MyApp app;
    return app.Run();
}
```

## Documentation

- [Xvfb Usage Guide](docs/Xvfb_Usage.md) - Running without display
- [WebGL Build Guide](docs/WebGL_Build.md) - Building for web
- [Architecture](roadmap/cross_platform_graphics_framework.md) - Framework design

## License

MIT License - see LICENSE file for details

## Contributing

Contributions are welcome! Please feel free to submit pull requests.

## Dependencies

- [LLGL](https://github.com/LukasBanana/LLGL) - Low Level Graphics Library
- [fmt](https://github.com/fmtlib/fmt) - Formatting library (header-only subset)

Dependencies are automatically fetched via CMake FetchContent.
