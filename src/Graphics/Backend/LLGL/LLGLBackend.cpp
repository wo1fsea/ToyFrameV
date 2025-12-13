/**
 * @file LLGLBackend.cpp
 * @brief LLGL implementation of the graphics backend interface
 */

#include "LLGLBackend.h"
#include "LLGLTypes.h"
#include "ToyFrameV/Core/Log.h"
#include <algorithm>

// LLGLSurfaceAdapter is only used on desktop platforms with native windows
// WebGL uses LLGL's built-in canvas management
#if !defined(__EMSCRIPTEN__) && !defined(PLATFORM_WEB)
#include "LLGLSurfaceAdapter.h"
#endif

namespace ToyFrameV {

// ============================================================================
// LLGLRenderTextureData Implementation
// ============================================================================

void LLGLRenderTextureData::Cleanup() {
    if (renderSystem) {
        // Cancel pending readbacks
        std::lock_guard<std::mutex> lock(readbackMutex);
        while (!pendingReadbacks.empty()) {
            auto& req = pendingReadbacks.front();
            if (req.stagingBuffer) {
                renderSystem->Release(*req.stagingBuffer);
            }
            pendingReadbacks.pop();
        }

        if (renderTarget) {
            renderSystem->Release(*renderTarget);
            renderTarget = nullptr;
        }
        if (colorTexture) {
            renderSystem->Release(*colorTexture);
            colorTexture = nullptr;
        }
        if (depthTexture) {
            renderSystem->Release(*depthTexture);
            depthTexture = nullptr;
        }
    }
}

bool LLGLRenderTextureData::Create(LLGL::RenderSystem* rs, uint32_t w, uint32_t h, PixelFormat fmt, bool depth) {
    renderSystem = rs;
    width = w;
    height = h;
    format = fmt;
    hasDepth = depth;
    
    // Create color texture
    LLGL::TextureDescriptor texDesc;
    texDesc.type = LLGL::TextureType::Texture2D;
    texDesc.extent = { width, height, 1 };
    texDesc.mipLevels = 1;
    texDesc.bindFlags = LLGL::BindFlags::Sampled | 
                        LLGL::BindFlags::ColorAttachment |
                        LLGL::BindFlags::CopySrc;
    texDesc.format = ToLLGLPixelFormat(format);

    colorTexture = renderSystem->CreateTexture(texDesc);
    if (!colorTexture) return false;

    // Create depth texture if needed
    if (hasDepth) {
        LLGL::TextureDescriptor depthDesc;
        depthDesc.type = LLGL::TextureType::Texture2D;
        depthDesc.extent = { width, height, 1 };
        depthDesc.mipLevels = 1;
        depthDesc.format = LLGL::Format::D32Float;
        depthDesc.bindFlags = LLGL::BindFlags::DepthStencilAttachment;

        depthTexture = renderSystem->CreateTexture(depthDesc);
    }

    // Create render target
    LLGL::RenderTargetDescriptor rtDesc;
    rtDesc.resolution = { width, height };
    rtDesc.colorAttachments[0] = colorTexture;
    if (depthTexture) {
        rtDesc.depthStencilAttachment = depthTexture;
    }

    renderTarget = renderSystem->CreateRenderTarget(rtDesc);
    return renderTarget != nullptr;
}

// ============================================================================
// LLGLBackend Implementation
// ============================================================================

LLGLBackend::LLGLBackend() = default;

LLGLBackend::~LLGLBackend() {
    Shutdown();
}

bool LLGLBackend::Initialize(Window* window, const BackendConfig& config) {
    m_toyFrameWindow = window;

    // Select backend module
    std::string moduleName;
    if (config.api == BackendConfig::API::Auto) {
#if defined(PLATFORM_WEB) || defined(__EMSCRIPTEN__)
        moduleName = "WebGL";
#elif defined(PLATFORM_WINDOWS)
        for (const auto& module : LLGL::RenderSystem::FindModules()) {
            if (module == "Direct3D11") {
                moduleName = module;
                break;
            }
        }
        if (moduleName.empty()) moduleName = "OpenGL";
#elif defined(PLATFORM_MACOS)
        moduleName = "Metal";
#else
        moduleName = "OpenGL";
#endif
    } else {
        const char* name = GetLLGLModuleName(config.api);
        if (name) moduleName = name;
        else moduleName = "OpenGL";
    }

    TOYFRAMEV_LOG_INFO("Initializing LLGL backend: {}", moduleName);

    // Load render system
    LLGL::RenderSystemDescriptor desc;
    desc.moduleName = moduleName;
    if (config.debugMode) {
        desc.flags = LLGL::RenderSystemFlags::DebugDevice;
    }

    LLGL::Report report;
    m_renderSystemPtr = LLGL::RenderSystem::Load(desc, &report);
    if (!m_renderSystemPtr) {
        TOYFRAMEV_LOG_ERROR("Failed to load render system: {}", report.GetText());
        return false;
    }
    m_renderSystem = m_renderSystemPtr.get();

    // Create swap chain
    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.samples = config.samples;

#if defined(__EMSCRIPTEN__) || defined(PLATFORM_WEB)
    // WebGL: LLGL manages the canvas directly, ignore external window parameter
    TOYFRAMEV_LOG_DEBUG("Using WebGL canvas");
    swapChainDesc.resolution = { config.windowWidth, config.windowHeight };
    m_swapChain = m_renderSystem->CreateSwapChain(swapChainDesc);
    m_ownsWindow = true;
#else
    // Desktop platforms: support external window via LLGLSurfaceAdapter
    if (window) {
        // Use external ToyFrameV window via adapter
        TOYFRAMEV_LOG_DEBUG("Using external window");
        m_externalSurface = CreateLLGLSurface(window);
        swapChainDesc.resolution = {
            static_cast<std::uint32_t>(window->GetWidth()),
            static_cast<std::uint32_t>(window->GetHeight())
        };
        m_swapChain = m_renderSystem->CreateSwapChain(swapChainDesc, m_externalSurface);
        m_ownsWindow = false;
    } else {
        // Let LLGL create its own window
        TOYFRAMEV_LOG_DEBUG("Creating LLGL window");
        swapChainDesc.resolution = { config.windowWidth, config.windowHeight };
        m_swapChain = m_renderSystem->CreateSwapChain(swapChainDesc);
        m_ownsWindow = true;

        // Get window from swap chain
        if (LLGL::IsInstanceOf<LLGL::Window>(m_swapChain->GetSurface())) {
            m_llglWindow = LLGL::CastTo<LLGL::Window>(&m_swapChain->GetSurface());
            m_llglWindow->Show();
        }
    }
#endif

    if (!m_swapChain) {
        TOYFRAMEV_LOG_ERROR("Failed to create swap chain");
        return false;
    }

    // Set vsync
    m_swapChain->SetVsyncInterval(config.vsync ? 1 : 0);

    // Create command buffer
    m_commandQueue = m_renderSystem->GetCommandQueue();
    m_commandBuffer = m_renderSystem->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);

    // Store info
    const auto& info = m_renderSystem->GetRendererInfo();
    m_backendName = info.rendererName;
    m_deviceName = info.deviceName;

    TOYFRAMEV_LOG_INFO("Renderer: {}", m_backendName);
    TOYFRAMEV_LOG_INFO("Device: {}", m_deviceName);

    return true;
}

void LLGLBackend::Shutdown() {
    if (m_renderSystemPtr) {
        LLGL::RenderSystem::Unload(std::move(m_renderSystemPtr));
        m_renderSystem = nullptr;
        m_swapChain = nullptr;
        m_commandBuffer = nullptr;
        m_commandQueue = nullptr;
        m_llglWindow = nullptr;
        m_externalSurface.reset();
    }
}

bool LLGLBackend::ProcessEvents() {
    // When using external window, we don't process LLGL window events
    // The ToyFrameV WindowSystem handles events for the external window
    if (m_ownsWindow) {
        // LLGL owns the window, process its events
        if (!LLGL::Surface::ProcessEvents()) {
            return false;
        }
        if (m_llglWindow && m_llglWindow->HasQuit()) {
            return false;
        }
    }
    // When using external window, just check if swap chain is valid
    return m_swapChain != nullptr;
}

bool LLGLBackend::IsValid() const {
    return m_renderSystem && m_swapChain != nullptr;
}

void LLGLBackend::BeginFrame() {
    m_commandBuffer->Begin();
    BeginRenderPassToCurrentTarget();
}

void LLGLBackend::EndFrame() {
    EndCurrentRenderPass();
    m_commandBuffer->End();
    
    // Only present if rendering to screen
    if (!m_currentRenderTarget) {
        m_swapChain->Present();
    }
}

void LLGLBackend::Clear(const Color& color) {
    const float clearColor[4] = { color.r, color.g, color.b, color.a };
    m_commandBuffer->Clear(LLGL::ClearFlags::Color, clearColor);
}

void LLGLBackend::OnResize(uint32_t width, uint32_t height) {
    if (m_swapChain && width > 0 && height > 0) {
        m_swapChain->ResizeBuffers({ width, height });
    }
}

LLGL::VertexFormat LLGLBackend::CreateVertexFormat(const VertexLayout& layout) {
    LLGL::VertexFormat format;
    for (const auto& attr : layout.attributes) {
        format.AppendAttribute({ attr.name.c_str(), ToLLGLFormat(attr.format) });
    }
    format.SetStride(layout.stride);
    return format;
}

void LLGLBackend::BeginRenderPassToCurrentTarget() {
    if (m_currentRenderTarget) {
        auto* rtData = static_cast<LLGLRenderTextureData*>(m_currentRenderTarget);
        if (rtData->renderTarget) {
            m_commandBuffer->SetViewport(rtData->renderTarget->GetResolution());
            m_commandBuffer->BeginRenderPass(*rtData->renderTarget);
        }
    } else {
        m_commandBuffer->SetViewport(m_swapChain->GetResolution());
        m_commandBuffer->BeginRenderPass(*m_swapChain);
    }
    m_inRenderPass = true;
}

void LLGLBackend::EndCurrentRenderPass() {
    if (m_inRenderPass) {
        m_commandBuffer->EndRenderPass();
        m_inRenderPass = false;
    }
}

// ==================== Resource Creation ====================

BackendHandle LLGLBackend::CreateBuffer(const BackendBufferDesc& desc) {
    LLGL::BufferDescriptor bufferDesc;
    bufferDesc.size = desc.size;

    // Keep vertex format alive until buffer is created
    LLGL::VertexFormat vertexFormat;

    switch (desc.type) {
        case BackendBufferDesc::Type::Vertex:
            bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
            if (!desc.vertexLayout.attributes.empty()) {
                vertexFormat = CreateVertexFormat(desc.vertexLayout);
                bufferDesc.vertexAttribs = vertexFormat.attributes;
            }
            break;
        case BackendBufferDesc::Type::Index:
            bufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
            break;
        case BackendBufferDesc::Type::Uniform:
            bufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
            break;
    }

    LLGL::Buffer* llglBuffer = m_renderSystem->CreateBuffer(bufferDesc, desc.initialData);
    return static_cast<BackendHandle>(llglBuffer);
}

void LLGLBackend::DestroyBuffer(BackendHandle buffer) {
    if (buffer && m_renderSystem) {
        m_renderSystem->Release(*static_cast<LLGL::Buffer*>(buffer));
    }
}

bool LLGLBackend::CreateShader(const BackendShaderDesc& desc,
                               BackendHandle& outVertexShader,
                               BackendHandle& outFragmentShader) {
    const auto& caps = m_renderSystem->GetRenderingCaps();
    const auto& languages = caps.shadingLanguages;

    bool useHLSL = std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end();
    
    // Create vertex format for input layout
    auto vertexFormat = CreateVertexFormat(desc.vertexLayout);

    // Create vertex shader
    LLGL::ShaderDescriptor vsDesc;
    vsDesc.type = LLGL::ShaderType::Vertex;
    vsDesc.source = desc.vertexShader.code.c_str();
    vsDesc.sourceType = LLGL::ShaderSourceType::CodeString;
    vsDesc.entryPoint = desc.vertexShader.entryPoint.c_str();
    vsDesc.profile = useHLSL ? "vs_4_0" : nullptr;
    vsDesc.vertex.inputAttribs = vertexFormat.attributes;

    LLGL::Shader* vertexShader = m_renderSystem->CreateShader(vsDesc);
    if (const auto* report = vertexShader->GetReport()) {
        if (report->HasErrors()) {
            TOYFRAMEV_LOG_ERROR("Vertex shader error: {}", report->GetText());
            return false;
        }
    }

    // Create fragment shader
    LLGL::ShaderDescriptor fsDesc;
    fsDesc.type = LLGL::ShaderType::Fragment;
    fsDesc.source = desc.fragmentShader.code.c_str();
    fsDesc.sourceType = LLGL::ShaderSourceType::CodeString;
    fsDesc.entryPoint = desc.fragmentShader.entryPoint.c_str();
    fsDesc.profile = useHLSL ? "ps_4_0" : nullptr;

    LLGL::Shader* fragmentShader = m_renderSystem->CreateShader(fsDesc);
    if (const auto* report = fragmentShader->GetReport()) {
        if (report->HasErrors()) {
            TOYFRAMEV_LOG_ERROR("Fragment shader error: {}", report->GetText());
            m_renderSystem->Release(*vertexShader);
            return false;
        }
    }

    outVertexShader = static_cast<BackendHandle>(vertexShader);
    outFragmentShader = static_cast<BackendHandle>(fragmentShader);
    return true;
}

void LLGLBackend::DestroyShader(BackendHandle vertexShader, BackendHandle fragmentShader) {
    if (m_renderSystem) {
        if (vertexShader) {
            m_renderSystem->Release(*static_cast<LLGL::Shader*>(vertexShader));
        }
        if (fragmentShader) {
            m_renderSystem->Release(*static_cast<LLGL::Shader*>(fragmentShader));
        }
    }
}

BackendHandle LLGLBackend::CreatePipeline(const BackendPipelineDesc& desc) {
    if (!desc.vertexShader || !desc.fragmentShader) {
        TOYFRAMEV_LOG_ERROR("Pipeline creation failed: shader is null");
        return nullptr;
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.vertexShader = static_cast<LLGL::Shader*>(desc.vertexShader);
    pipelineDesc.fragmentShader = static_cast<LLGL::Shader*>(desc.fragmentShader);
    pipelineDesc.renderPass = m_swapChain->GetRenderPass();

    // Rasterizer
    pipelineDesc.rasterizer.polygonMode = desc.wireframe ? LLGL::PolygonMode::Wireframe : LLGL::PolygonMode::Fill;
    pipelineDesc.rasterizer.cullMode = desc.cullBackFace ? LLGL::CullMode::Back : LLGL::CullMode::Disabled;

    // Depth
    pipelineDesc.depth.testEnabled = desc.depthTestEnabled;
    pipelineDesc.depth.writeEnabled = desc.depthWriteEnabled;

    // Blend
    if (desc.blendEnabled) {
        pipelineDesc.blend.targets[0].blendEnabled = true;
    }

    // Topology
    pipelineDesc.primitiveTopology = ToLLGLTopology(desc.topology);

    LLGL::PipelineState* pipelineState = m_renderSystem->CreatePipelineState(pipelineDesc);
    if (const auto* report = pipelineState->GetReport()) {
        if (report->HasErrors()) {
            TOYFRAMEV_LOG_ERROR("Pipeline error: {}", report->GetText());
            return nullptr;
        }
    }

    return static_cast<BackendHandle>(pipelineState);
}

void LLGLBackend::DestroyPipeline(BackendHandle pipeline) {
    if (pipeline && m_renderSystem) {
        m_renderSystem->Release(*static_cast<LLGL::PipelineState*>(pipeline));
    }
}

BackendHandle LLGLBackend::CreateRenderTexture(const BackendRenderTextureDesc& desc) {
    auto* rtData = new LLGLRenderTextureData();
    if (!rtData->Create(m_renderSystem, desc.width, desc.height, desc.format, desc.hasDepth)) {
        delete rtData;
        return nullptr;
    }
    return static_cast<BackendHandle>(rtData);
}

void LLGLBackend::DestroyRenderTexture(BackendHandle renderTexture) {
    if (renderTexture) {
        auto* rtData = static_cast<LLGLRenderTextureData*>(renderTexture);
        rtData->Cleanup();
        delete rtData;
    }
}

bool LLGLBackend::ResizeRenderTexture(BackendHandle renderTexture, uint32_t width, uint32_t height) {
    if (!renderTexture) return false;
    
    auto* rtData = static_cast<LLGLRenderTextureData*>(renderTexture);
    if (rtData->width == width && rtData->height == height) return true;
    
    // Store old settings
    bool hasDepth = rtData->hasDepth;
    PixelFormat format = rtData->format;
    auto* renderSystem = rtData->renderSystem;
    
    // Cleanup old resources
    rtData->Cleanup();
    
    // Recreate
    return rtData->Create(renderSystem, width, height, format, hasDepth);
}

// ==================== Render State ====================

void LLGLBackend::SetPipeline(BackendHandle pipeline) {
    m_commandBuffer->SetPipelineState(*static_cast<LLGL::PipelineState*>(pipeline));
}

void LLGLBackend::SetVertexBuffer(BackendHandle buffer) {
    m_commandBuffer->SetVertexBuffer(*static_cast<LLGL::Buffer*>(buffer));
}

void LLGLBackend::SetRenderTarget(BackendHandle renderTexture) {
    // End current render pass
    EndCurrentRenderPass();
    
    m_currentRenderTarget = renderTexture;
    
    // Begin new render pass
    BeginRenderPassToCurrentTarget();
}

// ==================== Drawing ====================

void LLGLBackend::Draw(uint32_t vertexCount, uint32_t firstVertex) {
    m_commandBuffer->Draw(vertexCount, firstVertex);
}

void LLGLBackend::DrawIndexed(uint32_t indexCount, uint32_t firstIndex) {
    m_commandBuffer->DrawIndexed(indexCount, firstIndex);
}

// ==================== Render Texture Operations ====================

BackendPixelData LLGLBackend::ReadRenderTexturePixels(BackendHandle renderTexture) {
    BackendPixelData result;
    if (!renderTexture) return result;
    
    auto* rtData = static_cast<LLGLRenderTextureData*>(renderTexture);
    if (!rtData->colorTexture || !rtData->renderSystem) return result;

    // CRITICAL: For WebGL, we must ensure all GPU commands are submitted
    // before reading the texture. End current render pass, submit commands,
    // then restart the render pass.
    bool wasInRenderPass = m_inRenderPass;
    if (wasInRenderPass) {
      EndCurrentRenderPass();
    }

    // Submit pending commands to GPU
    m_commandBuffer->End();
    m_commandQueue->Submit(*m_commandBuffer);
    m_commandQueue->WaitIdle(); // Wait for GPU to finish

    // Restart command buffer for any subsequent rendering
    m_commandBuffer->Begin();
    if (wasInRenderPass) {
      BeginRenderPassToCurrentTarget();
    }

    result.width = rtData->width;
    result.height = rtData->height;
    result.format = rtData->format;

    uint32_t bytesPerPixel = GetBytesPerPixel(rtData->format);
    result.data.resize(rtData->width * rtData->height * bytesPerPixel);

    // Use LLGL ReadTexture for synchronous readback
    LLGL::MutableImageView dstImage;
    dstImage.format = LLGL::ImageFormat::RGBA;
    dstImage.dataType = LLGL::DataType::UInt8;
    dstImage.data = result.data.data();
    dstImage.dataSize = result.data.size();

    LLGL::TextureRegion region;
    region.subresource = { 0, 1, 0, 1 };
    region.offset = { 0, 0, 0 };
    region.extent = { rtData->width, rtData->height, 1 };

    rtData->renderSystem->ReadTexture(*rtData->colorTexture, region, dstImage);

    return result;
}

void LLGLBackend::ReadRenderTexturePixelsAsync(BackendHandle renderTexture, BackendReadbackCallback callback) {
    if (!callback || !renderTexture) {
        if (callback) callback(BackendPixelData{});
        return;
    }
    
    auto* rtData = static_cast<LLGLRenderTextureData*>(renderTexture);
    if (!rtData->colorTexture || !rtData->renderSystem) {
        callback(BackendPixelData{});
        return;
    }

    // Create staging buffer for async readback
    uint32_t bytesPerPixel = GetBytesPerPixel(rtData->format);
    uint32_t dataSize = rtData->width * rtData->height * bytesPerPixel;

    LLGL::BufferDescriptor bufDesc;
    bufDesc.size = dataSize;
    bufDesc.bindFlags = 0;  // No binding, just for readback
    bufDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Read;
    bufDesc.miscFlags = LLGL::MiscFlags::NoInitialData;

    auto* stagingBuffer = rtData->renderSystem->CreateBuffer(bufDesc);
    if (!stagingBuffer) {
        callback(BackendPixelData{});
        return;
    }

    // Queue the readback request
    std::lock_guard<std::mutex> lock(rtData->readbackMutex);
    LLGLRenderTextureData::ReadbackRequest req;
    req.callback = callback;
    req.stagingBuffer = stagingBuffer;
    req.width = rtData->width;
    req.height = rtData->height;
    req.format = rtData->format;
    req.copyIssued = false;
    rtData->pendingReadbacks.push(req);
}

bool LLGLBackend::IsReadbackPending(BackendHandle renderTexture) const {
    if (!renderTexture) return false;
    
    auto* rtData = static_cast<LLGLRenderTextureData*>(renderTexture);
    std::lock_guard<std::mutex> lock(rtData->readbackMutex);
    return !rtData->pendingReadbacks.empty();
}

void LLGLBackend::CancelReadback(BackendHandle renderTexture) {
    if (!renderTexture) return;
    
    auto* rtData = static_cast<LLGLRenderTextureData*>(renderTexture);
    std::lock_guard<std::mutex> lock(rtData->readbackMutex);
    while (!rtData->pendingReadbacks.empty()) {
        auto& req = rtData->pendingReadbacks.front();
        if (req.stagingBuffer && rtData->renderSystem) {
            rtData->renderSystem->Release(*req.stagingBuffer);
        }
        rtData->pendingReadbacks.pop();
    }
}

// ============================================================================
// Factory Function
// ============================================================================

std::unique_ptr<IGraphicsBackend> CreateDefaultBackend() {
    return std::make_unique<LLGLBackend>();
}

} // namespace ToyFrameV
