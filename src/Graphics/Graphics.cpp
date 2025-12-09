/**
 * @file Graphics.cpp
 * @brief Graphics system implementation using LLGL
 * 
 * This file contains all LLGL-specific code. Users of ToyFrameV
 * never need to include LLGL headers.
 */

#include "ToyFrameV/Graphics.h"
#include "ToyFrameV/Graphics/RenderTexture.h"

// LLGLSurfaceAdapter is only used on desktop platforms with native windows
// WebGL uses LLGL's built-in canvas management
#if !defined(__EMSCRIPTEN__) && !defined(PLATFORM_WEB)
#include "LLGLSurfaceAdapter.h"
#endif

#include <LLGL/LLGL.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/VertexFormat.h>
#include <iostream>
#include <algorithm>
#include <vector>

namespace ToyFrameV {

// ============================================================================
// VertexLayout Implementation
// ============================================================================

uint32_t GetFormatSize(Format format) {
    switch (format) {
        case Format::Float:      return 4;
        case Format::Float2:     return 8;
        case Format::Float3:     return 12;
        case Format::Float4:     return 16;
        case Format::Int:        return 4;
        case Format::Int2:       return 8;
        case Format::Int3:       return 12;
        case Format::Int4:       return 16;
        case Format::UByte4Norm: return 4;
        case Format::UByte4:     return 4;
        default:                 return 0;
    }
}

static LLGL::Format ToLLGLFormat(Format format) {
    switch (format) {
        case Format::Float:      return LLGL::Format::R32Float;
        case Format::Float2:     return LLGL::Format::RG32Float;
        case Format::Float3:     return LLGL::Format::RGB32Float;
        case Format::Float4:     return LLGL::Format::RGBA32Float;
        case Format::Int:        return LLGL::Format::R32SInt;
        case Format::Int2:       return LLGL::Format::RG32SInt;
        case Format::Int3:       return LLGL::Format::RGB32SInt;
        case Format::Int4:       return LLGL::Format::RGBA32SInt;
        case Format::UByte4Norm: return LLGL::Format::RGBA8UNorm;
        case Format::UByte4:     return LLGL::Format::RGBA8UInt;
        default:                 return LLGL::Format::Undefined;
    }
}

VertexLayout& VertexLayout::Add(const std::string& name, Format format) {
    uint32_t offset = stride;
    attributes.push_back({ name, format, offset });
    stride += GetFormatSize(format);
    return *this;
}

void VertexLayout::CalculateOffsetsAndStride() {
    stride = 0;
    for (auto& attr : attributes) {
        attr.offset = stride;
        stride += GetFormatSize(attr.format);
    }
}

// ============================================================================
// GraphicsImpl - LLGL Implementation Details
// ============================================================================

class GraphicsImpl {
public:
    LLGL::RenderSystemPtr renderSystemPtr;
    LLGL::RenderSystem* renderSystem = nullptr;
    LLGL::SwapChain* swapChain = nullptr;
    LLGL::CommandBuffer* commandBuffer = nullptr;
    LLGL::CommandQueue* commandQueue = nullptr;
    LLGL::Window *window =
        nullptr; // Only used when LLGL creates its own window
    std::shared_ptr<LLGL::Surface>
        externalSurface;              // Adapter for external ToyFrameV window
    Window *toyFrameWindow = nullptr; // Reference to external ToyFrameV window
    bool inRenderPass = false;
    bool ownsWindow = false; // True if LLGL created the window
    
    // Current render target (nullptr = screen)
    RenderTexture* currentRenderTarget = nullptr;
    
    // Pending async readback requests from all RenderTextures
    std::vector<RenderTexture*> renderTexturesWithPendingReadbacks;

    ~GraphicsImpl() {
        if (renderSystemPtr) {
            LLGL::RenderSystem::Unload(std::move(renderSystemPtr));
        }
    }

    LLGL::VertexFormat CreateVertexFormat(const VertexLayout& layout) {
        LLGL::VertexFormat format;
        for (const auto& attr : layout.attributes) {
            format.AppendAttribute({ attr.name.c_str(), ToLLGLFormat(attr.format) });
        }
        format.SetStride(layout.stride);
        return format;
    }
};

// Helper function for RenderTexture to access LLGL render system
LLGL::RenderSystem* GetLLGLRenderSystem(GraphicsImpl* impl) {
    return impl ? impl->renderSystem : nullptr;
}

// ============================================================================
// Graphics Implementation
// ============================================================================

Graphics::~Graphics() = default;

std::unique_ptr<Graphics> Graphics::Create(Window* window, const GraphicsConfig& config) {
    auto graphics = std::unique_ptr<Graphics>(new Graphics());
    if (!graphics->Initialize(window, config)) {
        return nullptr;
    }
    return graphics;
}

bool Graphics::Initialize(Window* window, const GraphicsConfig& config) {
    m_impl = std::make_unique<GraphicsImpl>();
    m_impl->toyFrameWindow = window;

    // Select backend
    std::string moduleName;
    if (config.backend == GraphicsBackend::Auto) {
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
        switch (config.backend) {
            case GraphicsBackend::Direct3D11: moduleName = "Direct3D11"; break;
            case GraphicsBackend::Direct3D12: moduleName = "Direct3D12"; break;
            case GraphicsBackend::OpenGL:     moduleName = "OpenGL"; break;
            case GraphicsBackend::Vulkan:     moduleName = "Vulkan"; break;
            case GraphicsBackend::Metal:      moduleName = "Metal"; break;
            default: moduleName = "OpenGL"; break;
        }
    }

    std::cout << "Initializing graphics: " << moduleName << std::endl;

    // Load render system
    LLGL::RenderSystemDescriptor desc;
    desc.moduleName = moduleName;
    if (config.debugMode) {
        desc.flags = LLGL::RenderSystemFlags::DebugDevice;
    }

    LLGL::Report report;
    m_impl->renderSystemPtr = LLGL::RenderSystem::Load(desc, &report);
    if (!m_impl->renderSystemPtr) {
        std::cerr << "Failed to load render system: " << report.GetText() << std::endl;
        return false;
    }
    m_impl->renderSystem = m_impl->renderSystemPtr.get();

    // Create swap chain
    LLGL::SwapChainDescriptor swapChainDesc;
    swapChainDesc.samples = config.samples;

#if defined(__EMSCRIPTEN__) || defined(PLATFORM_WEB)
    // WebGL: LLGL manages the canvas directly, ignore external window parameter
    std::cout << "  Using WebGL canvas" << std::endl;
    swapChainDesc.resolution = {800, 600}; // Will be overridden by canvas size
    m_impl->swapChain = m_impl->renderSystem->CreateSwapChain(swapChainDesc);
    m_impl->ownsWindow = true;
#else
    // Desktop platforms: support external window via LLGLSurfaceAdapter
    if (window) {
      // Use external ToyFrameV window via adapter
      std::cout << "  Using external window" << std::endl;
      m_impl->externalSurface = CreateLLGLSurface(window);
      swapChainDesc.resolution = {
          static_cast<std::uint32_t>(window->GetWidth()),
          static_cast<std::uint32_t>(window->GetHeight())};
      m_impl->swapChain = m_impl->renderSystem->CreateSwapChain(
          swapChainDesc, m_impl->externalSurface);
      m_impl->ownsWindow = false;
    } else {
      // Let LLGL create its own window
      std::cout << "  Creating LLGL window" << std::endl;
      swapChainDesc.resolution = {800, 600};
      m_impl->swapChain = m_impl->renderSystem->CreateSwapChain(swapChainDesc);
      m_impl->ownsWindow = true;

      // Get window from swap chain
      if (LLGL::IsInstanceOf<LLGL::Window>(m_impl->swapChain->GetSurface())) {
        m_impl->window =
            LLGL::CastTo<LLGL::Window>(&m_impl->swapChain->GetSurface());
        m_impl->window->Show();
      }
    }
#endif

    if (!m_impl->swapChain) {
      std::cerr << "Failed to create swap chain" << std::endl;
      return false;
    }

    // Set vsync
    m_impl->swapChain->SetVsyncInterval(config.vsync ? 1 : 0);

    // Create command buffer
    m_impl->commandQueue = m_impl->renderSystem->GetCommandQueue();
    m_impl->commandBuffer = m_impl->renderSystem->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);

    // Store info
    const auto& info = m_impl->renderSystem->GetRendererInfo();
    m_backendName = info.rendererName;
    m_deviceName = info.deviceName;

    std::cout << "  Renderer: " << m_backendName << std::endl;
    std::cout << "  Device: " << m_deviceName << std::endl;

    return true;
}

// Forward declaration for RenderTextureImpl access
class RenderTextureImpl;
extern LLGL::RenderTarget* GetLLGLRenderTarget(RenderTextureImpl* impl);

void Graphics::BeginFrame() {
    m_impl->commandBuffer->Begin();
    
    // Begin render pass to current target (screen or RenderTexture)
    if (m_currentRenderTarget) {
        auto* rtImpl = m_currentRenderTarget->GetImpl();
        auto* llglRT = GetLLGLRenderTarget(rtImpl);
        if (llglRT) {
            m_impl->commandBuffer->SetViewport(llglRT->GetResolution());
            m_impl->commandBuffer->BeginRenderPass(*llglRT);
        }
    } else {
        m_impl->commandBuffer->SetViewport(m_impl->swapChain->GetResolution());
        m_impl->commandBuffer->BeginRenderPass(*m_impl->swapChain);
    }
    m_impl->inRenderPass = true;
}

void Graphics::EndFrame() {
    if (m_impl->inRenderPass) {
        m_impl->commandBuffer->EndRenderPass();
        m_impl->inRenderPass = false;
    }
    m_impl->commandBuffer->End();
    
    // Only present if rendering to screen
    if (!m_currentRenderTarget) {
        m_impl->swapChain->Present();
    }
}

void Graphics::Clear(const Color& color) {
    const float clearColor[4] = { color.r, color.g, color.b, color.a };
    m_impl->commandBuffer->Clear(LLGL::ClearFlags::Color, clearColor);
}

std::unique_ptr<Buffer> Graphics::CreateBuffer(const BufferDesc& desc) {
    LLGL::BufferDescriptor bufferDesc;
    bufferDesc.size = desc.size;

    // Keep vertex format alive until buffer is created
    // (bufferDesc.vertexAttribs is an ArrayView, not a copy!)
    LLGL::VertexFormat vertexFormat;

    switch (desc.type) {
        case BufferType::Vertex:
            bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
            if (!desc.vertexLayout.attributes.empty()) {
                vertexFormat = m_impl->CreateVertexFormat(desc.vertexLayout);
                bufferDesc.vertexAttribs = vertexFormat.attributes;
            }
            break;
        case BufferType::Index:
            bufferDesc.bindFlags = LLGL::BindFlags::IndexBuffer;
            break;
        case BufferType::Uniform:
            bufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
            break;
    }

    LLGL::Buffer* llglBuffer = m_impl->renderSystem->CreateBuffer(bufferDesc, desc.initialData);
    if (!llglBuffer) {
        return nullptr;
    }

    auto buffer = std::unique_ptr<Buffer>(new Buffer());
    buffer->m_handle = llglBuffer;
    buffer->m_type = desc.type;
    buffer->m_size = desc.size;
    buffer->m_graphics = this;
    return buffer;
}

std::unique_ptr<Shader> Graphics::CreateShader(const ShaderDesc& desc) {
    auto* rs = m_impl->renderSystem;
    const auto& caps = rs->GetRenderingCaps();
    const auto& languages = caps.shadingLanguages;

    bool useHLSL = std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end();
    
    // Create vertex format for input layout
    auto vertexFormat = m_impl->CreateVertexFormat(desc.vertexLayout);

    // Create vertex shader
    LLGL::ShaderDescriptor vsDesc;
    vsDesc.type = LLGL::ShaderType::Vertex;
    vsDesc.source = desc.vertexShader.code.c_str();
    vsDesc.sourceType = LLGL::ShaderSourceType::CodeString;
    vsDesc.entryPoint = desc.vertexShader.entryPoint.c_str();
    vsDesc.profile = useHLSL ? "vs_4_0" : nullptr;
    vsDesc.vertex.inputAttribs = vertexFormat.attributes;

    LLGL::Shader* vertexShader = rs->CreateShader(vsDesc);
    if (const auto* report = vertexShader->GetReport()) {
        if (report->HasErrors()) {
            std::cerr << "Vertex shader error: " << report->GetText() << std::endl;
            return nullptr;
        }
    }

    // Create fragment shader
    LLGL::ShaderDescriptor fsDesc;
    fsDesc.type = LLGL::ShaderType::Fragment;
    fsDesc.source = desc.fragmentShader.code.c_str();
    fsDesc.sourceType = LLGL::ShaderSourceType::CodeString;
    fsDesc.entryPoint = desc.fragmentShader.entryPoint.c_str();
    fsDesc.profile = useHLSL ? "ps_4_0" : nullptr;

    LLGL::Shader* fragmentShader = rs->CreateShader(fsDesc);
    if (const auto* report = fragmentShader->GetReport()) {
        if (report->HasErrors()) {
            std::cerr << "Fragment shader error: " << report->GetText() << std::endl;
            return nullptr;
        }
    }

    auto shader = std::unique_ptr<Shader>(new Shader());
    shader->m_vertexHandle = vertexShader;
    shader->m_fragmentHandle = fragmentShader;
    shader->m_graphics = this;
    return shader;
}

std::unique_ptr<Pipeline> Graphics::CreatePipeline(const PipelineDesc& desc) {
    if (!desc.shader) {
        std::cerr << "Pipeline creation failed: shader is null" << std::endl;
        return nullptr;
    }

    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    pipelineDesc.vertexShader = static_cast<LLGL::Shader*>(desc.shader->GetVertexHandle());
    pipelineDesc.fragmentShader = static_cast<LLGL::Shader*>(desc.shader->GetFragmentHandle());
    pipelineDesc.renderPass = m_impl->swapChain->GetRenderPass();

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
    switch (desc.topology) {
        case Topology::TriangleList:  pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList; break;
        case Topology::TriangleStrip: pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip; break;
        case Topology::LineList:      pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::LineList; break;
        case Topology::LineStrip:     pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::LineStrip; break;
        case Topology::PointList:     pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::PointList; break;
    }

    LLGL::PipelineState* pipelineState = m_impl->renderSystem->CreatePipelineState(pipelineDesc);
    if (const auto* report = pipelineState->GetReport()) {
        if (report->HasErrors()) {
            std::cerr << "Pipeline error: " << report->GetText() << std::endl;
            return nullptr;
        }
    }

    auto pipeline = std::unique_ptr<Pipeline>(new Pipeline());
    pipeline->m_handle = pipelineState;
    pipeline->m_graphics = this;
    return pipeline;
}

void Graphics::SetPipeline(Pipeline* pipeline) {
    m_impl->commandBuffer->SetPipelineState(*static_cast<LLGL::PipelineState*>(pipeline->GetHandle()));
}

void Graphics::SetVertexBuffer(Buffer* buffer) {
    m_impl->commandBuffer->SetVertexBuffer(*static_cast<LLGL::Buffer*>(buffer->GetHandle()));
}

void Graphics::Draw(uint32_t vertexCount, uint32_t firstVertex) {
    m_impl->commandBuffer->Draw(vertexCount, firstVertex);
}

void Graphics::DrawIndexed(uint32_t indexCount, uint32_t firstIndex) {
    m_impl->commandBuffer->DrawIndexed(indexCount, firstIndex);
}

const std::string& Graphics::GetBackendName() const {
    return m_backendName;
}

const std::string& Graphics::GetDeviceName() const {
    return m_deviceName;
}

Window* Graphics::GetWindow() const {
  return m_impl->toyFrameWindow; // Return external ToyFrameV window if provided
}

void Graphics::OnResize(int width, int height) {
    if (m_impl->swapChain && width > 0 && height > 0) {
        m_impl->swapChain->ResizeBuffers({ static_cast<uint32_t>(width), static_cast<uint32_t>(height) });
    }
}

bool Graphics::ProcessEvents() {
  // When using external window, we don't process LLGL window events
  // The ToyFrameV WindowSystem handles events for the external window
  if (m_impl->ownsWindow) {
    // LLGL owns the window, process its events
    if (!LLGL::Surface::ProcessEvents()) {
      return false;
    }
    if (m_impl->window && m_impl->window->HasQuit()) {
      return false;
    }
  }
    // When using external window, just check if swap chain is valid
    return m_impl->swapChain != nullptr;
}

bool Graphics::IsValid() const {
  return m_impl && m_impl->swapChain != nullptr;
}

// ============================================================================
// RenderTexture Support
// ============================================================================

std::unique_ptr<RenderTexture> Graphics::CreateRenderTexture(const RenderTextureDesc& desc) {
    auto rt = std::unique_ptr<RenderTexture>(new RenderTexture(this));
    if (!rt->Initialize(desc)) {
        return nullptr;
    }
    return rt;
}

void Graphics::SetRenderTarget(RenderTexture* rt) {
    // If we're in a render pass, end it first
    if (m_impl->inRenderPass) {
        m_impl->commandBuffer->EndRenderPass();
        m_impl->inRenderPass = false;
    }
    
    m_currentRenderTarget = rt;
    m_impl->currentRenderTarget = rt;
    
    // Begin new render pass to the new target
    if (rt) {
        auto* rtImpl = rt->GetImpl();
        auto* llglRT = GetLLGLRenderTarget(rtImpl);
        if (llglRT) {
            m_impl->commandBuffer->SetViewport(llglRT->GetResolution());
            m_impl->commandBuffer->BeginRenderPass(*llglRT);
            m_impl->inRenderPass = true;
        }
    } else {
        // Back to screen
        m_impl->commandBuffer->SetViewport(m_impl->swapChain->GetResolution());
        m_impl->commandBuffer->BeginRenderPass(*m_impl->swapChain);
        m_impl->inRenderPass = true;
    }
}

RenderTexture* Graphics::GetRenderTarget() const {
    return m_currentRenderTarget;
}

void Graphics::ProcessReadbacks() {
    // Process async readback callbacks
    // For now, this is a stub - async readback will be implemented
    // when we add proper fence/query support
}

// ============================================================================
// Buffer Implementation
// ============================================================================

Buffer::~Buffer() {
    // LLGL resources are cleaned up when RenderSystem is unloaded
}

// ============================================================================
// Shader Implementation
// ============================================================================

Shader::~Shader() {
    // LLGL resources are cleaned up when RenderSystem is unloaded
}

// ============================================================================
// Pipeline Implementation
// ============================================================================

Pipeline::~Pipeline() {
    // LLGL resources are cleaned up when RenderSystem is unloaded
}

} // namespace ToyFrameV
