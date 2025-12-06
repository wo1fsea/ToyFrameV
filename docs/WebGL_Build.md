# ToyFrameV WebGL 构建指南

ToyFrameV 支持编译为 WebAssembly，在浏览器中通过 WebGL 运行。

## 前提条件

### 安装 Emscripten SDK

```bash
# 克隆 emsdk
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 安装最新版本
./emsdk install latest
./emsdk activate latest

# 激活环境变量
# Linux/macOS:
source ./emsdk_env.sh
# Windows:
emsdk_env.bat
```

## 构建步骤

### Linux/macOS

```bash
cd ToyFrameV
chmod +x build_web.sh
./build_web.sh
```

### Windows (PowerShell)

```powershell
cd ToyFrameV
.\build_web.ps1
```

### 手动构建

```bash
mkdir build-web
cd build-web
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release
emmake make -j4
```

## 测试

构建完成后，在 `build-web/bin/` 目录下会生成：

- `HelloTriangle.html` - 主 HTML 文件
- `HelloTriangle.js` - JavaScript 胶水代码
- `HelloTriangle.wasm` - WebAssembly 模块

启动本地服务器测试：

```bash
cd build-web/bin
python3 -m http.server 8080
```

然后在浏览器中打开: http://localhost:8080/HelloTriangle.html

## 浏览器兼容性

需要支持 WebGL 2.0 的现代浏览器：

- Chrome 56+
- Firefox 51+
- Safari 15+
- Edge 79+

## 着色器

WebGL 使用 GLSL ES 3.0 着色器。HelloTriangle 示例已包含 HLSL 和 GLSL 两种着色器，会根据后端自动选择。

```cpp
// 示例：根据后端选择着色器
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

## 注意事项

1. WebGL 构建会生成较大的 WASM 文件，首次加载可能需要几秒钟
2. 确保服务器正确设置 MIME 类型：`application/wasm` 用于 `.wasm` 文件
3. 部分浏览器可能需要 HTTPS 才能运行 WebAssembly
