# ToyFrameV - 跨平台图形框架开发路线图

## 📊 项目概述

ToyFrameV 是一个基于 LLGL 的轻量级跨平台图形框架，支持 Desktop 和 Web 平台。

## ✅ 目标平台
- **Desktop**: Windows ✅ | macOS (计划中) | Linux (计划中)
- **Mobile**: iOS (计划中) | Android (计划中)
- **Web**: WebAssembly + WebGL ✅

## 🔧 技术栈
- **构建系统**: CMake + Ninja (Web)
- **图形后端**: [LLGL](https://github.com/LukasBanana/LLGL) (通过 FetchContent 自动获取)
- **Web 工具链**: Emscripten SDK

---

## 📂 当前项目结构

```
ToyFrameV/
├── CMakeLists.txt              # 主 CMake 配置
├── cmake/Platform.cmake        # 平台检测
├── build_web.ps1/sh           # Web 构建脚本
├── include/ToyFrameV/          # 公共头文件
│   ├── App.h                   # 应用生命周期
│   ├── Window.h                # 窗口抽象
│   ├── Input.h                 # 输入系统
│   ├── KeyCodes.h              # 键码定义
│   ├── Graphics.h              # 图形渲染
│   └── Platform.h              # 平台抽象
├── src/
│   ├── App.cpp                 # App 实现
│   ├── Input.cpp               # 输入核心实现
│   ├── Window/WindowWindows.cpp
│   ├── Input/InputWindows.cpp
│   ├── Graphics/Graphics.cpp   # LLGL 渲染封装
│   └── Platform/
│       ├── Windows/PlatformWindows.cpp
│       └── Web/PlatformWeb.cpp
├── samples/
│   ├── HelloApp/               # 基础应用示例
│   └── HelloTriangle/          # 三角形渲染示例
├── web/template.html           # Web 构建模板
└── docs/WebGL_Build.md         # Web 构建文档
```

---

## 🗂️ 开发阶段

### ✅ Stage 1: 项目初始化 (已完成)
- [x] CMake 项目结构搭建
- [x] LLGL 通过 FetchContent 集成
- [x] 平台检测宏 (`cmake/Platform.cmake`)
- [x] 基础 `App` 类和入口点

### ✅ Stage 2: 窗口创建 (已完成)
- [x] Windows 平台窗口 (`WindowWindows.cpp`)
- [x] LLGL SwapChain 集成
- [x] Web 平台 Canvas 支持
- [x] HelloApp 示例可运行

### ✅ Stage 3: 输入系统 (已完成)
- [x] 统一 `Input` 接口 (`Input.h`)
- [x] 键码定义 (`KeyCodes.h`)
- [x] Windows 键盘/鼠标输入 (`InputWindows.cpp`)
- [x] 基础输入事件回调

### ✅ Stage 4: 基础渲染 (已完成)
- [x] `Graphics` 类封装 LLGL 渲染器
- [x] 顶点缓冲区创建
- [x] 着色器加载 (HLSL/GLSL/SPIRV)
- [x] 渲染管线配置
- [x] **HelloTriangle 示例**
  - [x] Windows (Direct3D 11) ✅
  - [x] Web (WebGL) ✅

### ✅ Stage 5: Web 平台支持 (已完成)
- [x] Emscripten 工具链集成
- [x] `build_web.ps1` / `build_web.sh` 构建脚本
- [x] Web 主循环 (`emscripten_set_main_loop`)
- [x] HTML 模板 (`web/template.html`)
- [x] WebGL 着色器兼容

---

## 🚧 下一阶段任务

### 📋 Stage 6: 跨平台扩展
- [ ] **macOS 支持**
  - [ ] Metal 后端测试
  - [ ] Cocoa 窗口创建
  - [ ] 输入事件处理
- [ ] **Linux 支持**
  - [ ] Vulkan/OpenGL 后端
  - [ ] X11/Wayland 窗口
  - [ ] 输入事件处理

### 📋 Stage 7: 功能增强
- [ ] **纹理系统**
  - [ ] 纹理加载 (PNG/JPG)
  - [ ] 纹理采样器
  - [ ] 带纹理的四边形渲染
- [ ] **统一缓冲区 (Uniform Buffer)**
  - [ ] MVP 矩阵传递
  - [ ] 时间/分辨率等全局参数
- [ ] **数学库**
  - [ ] 向量/矩阵运算
  - [ ] 变换工具函数

### 📋 Stage 8: 移动端支持
- [ ] **Android**
  - [ ] NDK 构建配置
  - [ ] EGL/OpenGL ES 后端
  - [ ] 触摸输入
- [ ] **iOS**
  - [ ] Xcode 项目生成
  - [ ] Metal 后端
  - [ ] 触摸输入

### 📋 Stage 9: 高级功能 (远期)
- [ ] ImGui 集成
- [ ] 多 Pass 渲染
- [ ] 资源管理器
- [ ] 场景图系统
- [ ] 音频系统

---

## ✅ 设计原则
- 用户代码中无 `#ifdef`，平台差异封装在实现层
- 模块化设计：`App`、`Window`、`Input`、`Graphics` 可独立使用
- LLGL 作为唯一图形抽象层
- 尽量减少第三方依赖

---

## 📝 构建指南

### Windows (Visual Studio)
```powershell
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

### Web (Emscripten)
```powershell
.\build_web.ps1        # 首次构建
.\build_web.ps1 -Clean # 清理重建
```

构建产物位于 `build-web/bin/`，使用本地 HTTP 服务器运行 HTML 文件。

---

*最后更新: 2025年12月6日*

