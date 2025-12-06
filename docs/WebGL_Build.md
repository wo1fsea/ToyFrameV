# ToyFrameV WebGL Build Guide

ToyFrameV supports compilation to WebAssembly, running in browsers via WebGL.

## Prerequisites

### Install Emscripten SDK

```bash
# Clone emsdk
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install latest version
./emsdk install latest
./emsdk activate latest

# Activate environment variables
# Linux/macOS:
source ./emsdk_env.sh
# Windows:
emsdk_env.bat
```

## Build Steps

### Linux/macOS

```bash
cd ToyFrameV
chmod +x scripts/build_web.sh
./scripts/build_web.sh
```

### Windows (PowerShell)

```powershell
cd ToyFrameV
.\scripts\build_web.ps1
```

### Manual Build

```bash
mkdir build-web
cd build-web
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j4
```

## Testing

After building, the following files will be generated in `build-web/bin/`:

- `HelloTriangle.html` - Main HTML file
- `HelloTriangle.js` - JavaScript glue code
- `HelloTriangle.wasm` - WebAssembly module

Start a local server to test:

```bash
cd build-web/bin
python3 -m http.server 8080
```

Then open in browser: http://localhost:8080/HelloTriangle.html

## Browser Compatibility

Requires modern browsers with WebGL 2.0 support:

- Chrome 56+
- Firefox 51+
- Safari 15+
- Edge 79+

## Shaders

WebGL uses GLSL ES 3.0 shaders. The HelloTriangle sample includes both HLSL and GLSL shaders, automatically selected based on the backend.

```cpp
// Example: Select shader based on backend
bool useGLSL = (gfx->GetBackendName().find("OpenGL") != std::string::npos) ||
               (gfx->GetBackendName().find("WebGL") != std::string::npos);

if (useGLSL) {
    shaderDesc.vertexShader = { ShaderStage::Vertex, g_vertexShaderGLSL, "main" };
    shaderDesc.fragmentShader = { ShaderStage::Fragment, g_fragmentShaderGLSL, "main" };
} else {
    shaderDesc.vertexShader = { ShaderStage::Vertex, g_vertexShaderHLSL, "VS" };
    shaderDesc.fragmentShader = { ShaderStage::Fragment, g_fragmentShaderHLSL, "PS" };
}
```

## Notes

1. WebGL builds generate larger WASM files; initial loading may take a few seconds
2. Ensure the server correctly sets MIME types: `application/wasm` for `.wasm` files
3. Some browsers may require HTTPS to run WebAssembly
