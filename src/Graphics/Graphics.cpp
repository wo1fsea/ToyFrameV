/**
 * @file Graphics.cpp
 * @brief Graphics system implementation using backend abstraction
 *
 * This file no longer contains any LLGL-specific code.
 * All rendering operations are delegated to IGraphicsBackend.
 */

#include "ToyFrameV/Graphics.h"
#include "Backend/IGraphicsBackend.h"
#include "ToyFrameV/Core/Log.h"
#include "ToyFrameV/Graphics/RenderTexture.h"

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
// GraphicsImpl - Backend Wrapper
// ============================================================================

class GraphicsImpl {
public:
  std::unique_ptr<IGraphicsBackend> backend;

  ~GraphicsImpl() = default;
};

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

  // Create the default backend (LLGL)
  m_impl->backend = CreateDefaultBackend();
  if (!m_impl->backend) {
    TOYFRAMEV_LOG_ERROR("Failed to create graphics backend");
    return false;
  }

  // Convert GraphicsConfig to BackendConfig
  BackendConfig backendConfig;
  switch (config.backend) {
  case GraphicsBackend::Auto:
    backendConfig.api = BackendConfig::API::Auto;
    break;
  case GraphicsBackend::Direct3D11:
    backendConfig.api = BackendConfig::API::Direct3D11;
    break;
  case GraphicsBackend::Direct3D12:
    backendConfig.api = BackendConfig::API::Direct3D12;
    break;
  case GraphicsBackend::OpenGL:
    backendConfig.api = BackendConfig::API::OpenGL;
    break;
  case GraphicsBackend::Vulkan:
    backendConfig.api = BackendConfig::API::Vulkan;
    break;
  case GraphicsBackend::Metal:
    backendConfig.api = BackendConfig::API::Metal;
    break;
  }
    backendConfig.vsync = config.vsync;
    backendConfig.samples = config.samples;
    backendConfig.debugMode = config.debugMode;
    if (window) {
      backendConfig.windowWidth = static_cast<uint32_t>(window->GetWidth());
      backendConfig.windowHeight = static_cast<uint32_t>(window->GetHeight());
    }

    // Initialize backend
    if (!m_impl->backend->Initialize(window, backendConfig)) {
      return false;
    }

    // Store info from backend
    m_backendName = m_impl->backend->GetBackendName();
    m_deviceName = m_impl->backend->GetDeviceName();

    return true;
}

void Graphics::BeginFrame() { m_impl->backend->BeginFrame(); }

void Graphics::EndFrame() { m_impl->backend->EndFrame(); }

void Graphics::Clear(const Color &color) { m_impl->backend->Clear(color); }

std::unique_ptr<Buffer> Graphics::CreateBuffer(const BufferDesc& desc) {
  BackendBufferDesc backendDesc;
  switch (desc.type) {
  case BufferType::Vertex:
    backendDesc.type = BackendBufferDesc::Type::Vertex;
    break;
  case BufferType::Index:
    backendDesc.type = BackendBufferDesc::Type::Index;
    break;
  case BufferType::Uniform:
    backendDesc.type = BackendBufferDesc::Type::Uniform;
    break;
  }
    backendDesc.size = desc.size;
    backendDesc.initialData = desc.initialData;
    backendDesc.vertexLayout = desc.vertexLayout;

    BackendHandle handle = m_impl->backend->CreateBuffer(backendDesc);
    if (!handle) {
      return nullptr;
    }

    auto buffer = std::unique_ptr<Buffer>(new Buffer());
    buffer->m_handle = handle;
    buffer->m_type = desc.type;
    buffer->m_size = desc.size;
    buffer->m_graphics = this;
    return buffer;
}

std::unique_ptr<Shader> Graphics::CreateShader(const ShaderDesc& desc) {
  BackendShaderDesc backendDesc;
  backendDesc.vertexShader.code = desc.vertexShader.code;
  backendDesc.vertexShader.entryPoint = desc.vertexShader.entryPoint;
  backendDesc.fragmentShader.code = desc.fragmentShader.code;
  backendDesc.fragmentShader.entryPoint = desc.fragmentShader.entryPoint;
  backendDesc.vertexLayout = desc.vertexLayout;

  BackendHandle vertexHandle = nullptr;
  BackendHandle fragmentHandle = nullptr;

  if (!m_impl->backend->CreateShader(backendDesc, vertexHandle,
                                     fragmentHandle)) {
    return nullptr;
  }

    auto shader = std::unique_ptr<Shader>(new Shader());
    shader->m_vertexHandle = vertexHandle;
    shader->m_fragmentHandle = fragmentHandle;
    shader->m_graphics = this;
    return shader;
}

std::unique_ptr<Pipeline> Graphics::CreatePipeline(const PipelineDesc& desc) {
    if (!desc.shader) {
        TOYFRAMEV_LOG_ERROR("Pipeline creation failed: shader is null");
        return nullptr;
    }

    BackendPipelineDesc backendDesc;
    backendDesc.vertexShader = desc.shader->GetVertexHandle();
    backendDesc.fragmentShader = desc.shader->GetFragmentHandle();
    backendDesc.topology = desc.topology;
    backendDesc.wireframe = desc.wireframe;
    backendDesc.cullBackFace = desc.cullBackFace;
    backendDesc.depthTestEnabled = desc.depthTestEnabled;
    backendDesc.depthWriteEnabled = desc.depthWriteEnabled;
    backendDesc.blendEnabled = desc.blendEnabled;

    BackendHandle handle = m_impl->backend->CreatePipeline(backendDesc);
    if (!handle) {
      return nullptr;
    }

    auto pipeline = std::unique_ptr<Pipeline>(new Pipeline());
    pipeline->m_handle = handle;
    pipeline->m_graphics = this;
    return pipeline;
}

void Graphics::SetPipeline(Pipeline* pipeline) {
  m_impl->backend->SetPipeline(pipeline->GetHandle());
}

void Graphics::SetVertexBuffer(Buffer* buffer) {
  m_impl->backend->SetVertexBuffer(buffer->GetHandle());
}

void Graphics::Draw(uint32_t vertexCount, uint32_t firstVertex) {
  m_impl->backend->Draw(vertexCount, firstVertex);
}

void Graphics::DrawIndexed(uint32_t indexCount, uint32_t firstIndex) {
  m_impl->backend->DrawIndexed(indexCount, firstIndex);
}

const std::string& Graphics::GetBackendName() const {
    return m_backendName;
}

const std::string& Graphics::GetDeviceName() const {
    return m_deviceName;
}

Window *Graphics::GetWindow() const { return m_impl->backend->GetWindow(); }

void Graphics::OnResize(int width, int height) {
  if (width > 0 && height > 0) {
    m_impl->backend->OnResize(static_cast<uint32_t>(width),
                              static_cast<uint32_t>(height));
  }
}

bool Graphics::ProcessEvents() { return m_impl->backend->ProcessEvents(); }

bool Graphics::IsValid() const {
  return m_impl && m_impl->backend && m_impl->backend->IsValid();
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

void Graphics::SetRenderTarget(RenderTexture *rt) {
  if (rt) {
    m_impl->backend->SetRenderTarget(rt->GetBackendHandle());
  } else {
    m_impl->backend->SetRenderTarget(nullptr);
  }
  m_currentRenderTarget = rt;
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
// Internal Backend Access
// ============================================================================

IGraphicsBackend *Graphics::GetBackend() const {
  return m_impl ? m_impl->backend.get() : nullptr;
}

// ============================================================================
// Buffer Implementation
// ============================================================================

Buffer::~Buffer() {
  // Note: Backend resources are managed by the backend
  // They will be cleaned up when the backend is destroyed
  // For explicit cleanup, we would need to call backend->DestroyBuffer()
}

// ============================================================================
// Shader Implementation
// ============================================================================

Shader::~Shader() {
  // Note: Backend resources are managed by the backend
}

// ============================================================================
// Pipeline Implementation
// ============================================================================

Pipeline::~Pipeline() {
  // Note: Backend resources are managed by the backend
}

} // namespace ToyFrameV
