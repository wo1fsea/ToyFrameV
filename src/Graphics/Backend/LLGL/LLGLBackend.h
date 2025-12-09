/**
 * @file LLGLBackend.h
 * @brief LLGL implementation of the graphics backend interface
 * 
 * This file contains all LLGL-specific code. Users of ToyFrameV
 * never need to include LLGL headers.
 */

#pragma once

#include "../IGraphicsBackend.h"
#include <LLGL/LLGL.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/VertexFormat.h>
#include <memory>
#include <vector>
#include <queue>
#include <mutex>

namespace ToyFrameV {

/**
 * @brief LLGL implementation of IGraphicsBackend
 */
class LLGLBackend : public IGraphicsBackend {
public:
    LLGLBackend();
    ~LLGLBackend() override;

    // Non-copyable
    LLGLBackend(const LLGLBackend&) = delete;
    LLGLBackend& operator=(const LLGLBackend&) = delete;

    // ==================== Lifecycle ====================
    bool Initialize(Window* window, const BackendConfig& config) override;
    void Shutdown() override;
    bool ProcessEvents() override;
    bool IsValid() const override;

    // ==================== Frame Management ====================
    void BeginFrame() override;
    void EndFrame() override;
    void Clear(const Color& color) override;
    void OnResize(uint32_t width, uint32_t height) override;

    // ==================== Resource Creation ====================
    BackendHandle CreateBuffer(const BackendBufferDesc& desc) override;
    void DestroyBuffer(BackendHandle buffer) override;
    bool CreateShader(const BackendShaderDesc& desc, 
                      BackendHandle& outVertexShader, 
                      BackendHandle& outFragmentShader) override;
    void DestroyShader(BackendHandle vertexShader, BackendHandle fragmentShader) override;
    BackendHandle CreatePipeline(const BackendPipelineDesc& desc) override;
    void DestroyPipeline(BackendHandle pipeline) override;
    BackendHandle CreateRenderTexture(const BackendRenderTextureDesc& desc) override;
    void DestroyRenderTexture(BackendHandle renderTexture) override;
    bool ResizeRenderTexture(BackendHandle renderTexture, uint32_t width, uint32_t height) override;

    // ==================== Render State ====================
    void SetPipeline(BackendHandle pipeline) override;
    void SetVertexBuffer(BackendHandle buffer) override;
    void SetRenderTarget(BackendHandle renderTexture) override;

    // ==================== Drawing ====================
    void Draw(uint32_t vertexCount, uint32_t firstVertex) override;
    void DrawIndexed(uint32_t indexCount, uint32_t firstIndex) override;

    // ==================== Render Texture Operations ====================
    BackendPixelData ReadRenderTexturePixels(BackendHandle renderTexture) override;
    void ReadRenderTexturePixelsAsync(BackendHandle renderTexture, BackendReadbackCallback callback) override;
    bool IsReadbackPending(BackendHandle renderTexture) const override;
    void CancelReadback(BackendHandle renderTexture) override;

    // ==================== Queries ====================
    const std::string& GetBackendName() const override { return m_backendName; }
    const std::string& GetDeviceName() const override { return m_deviceName; }
    Window* GetWindow() const override { return m_toyFrameWindow; }
    bool OwnsWindow() const override { return m_ownsWindow; }

    // ==================== LLGL-Specific Accessors ====================
    LLGL::RenderSystem* GetRenderSystem() const { return m_renderSystem; }
    LLGL::SwapChain* GetSwapChain() const { return m_swapChain; }
    LLGL::CommandBuffer* GetCommandBuffer() const { return m_commandBuffer; }

private:
    // Helper methods
    LLGL::VertexFormat CreateVertexFormat(const VertexLayout& layout);
    void BeginRenderPassToCurrentTarget();
    void EndCurrentRenderPass();

    // LLGL objects
    LLGL::RenderSystemPtr m_renderSystemPtr;
    LLGL::RenderSystem* m_renderSystem = nullptr;
    LLGL::SwapChain* m_swapChain = nullptr;
    LLGL::CommandBuffer* m_commandBuffer = nullptr;
    LLGL::CommandQueue* m_commandQueue = nullptr;
    LLGL::Window* m_llglWindow = nullptr;  // Only used when LLGL creates its own window
    
    // External window adapter
    std::shared_ptr<LLGL::Surface> m_externalSurface;
    Window* m_toyFrameWindow = nullptr;
    
    // State
    bool m_inRenderPass = false;
    bool m_ownsWindow = false;
    BackendHandle m_currentRenderTarget = nullptr;
    
    // Info strings
    std::string m_backendName;
    std::string m_deviceName;
};

/**
 * @brief Internal render texture data for LLGL backend
 */
struct LLGLRenderTextureData {
    LLGL::Texture* colorTexture = nullptr;
    LLGL::Texture* depthTexture = nullptr;
    LLGL::RenderTarget* renderTarget = nullptr;
    LLGL::RenderSystem* renderSystem = nullptr;
    
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    bool hasDepth = true;

    // Async readback state
    struct ReadbackRequest {
        BackendReadbackCallback callback;
        LLGL::Buffer* stagingBuffer = nullptr;
        uint32_t width = 0;
        uint32_t height = 0;
        PixelFormat format = PixelFormat::RGBA8;
        bool copyIssued = false;
    };
    std::queue<ReadbackRequest> pendingReadbacks;
    mutable std::mutex readbackMutex;

    void Cleanup();
    bool Create(LLGL::RenderSystem* rs, uint32_t w, uint32_t h, PixelFormat fmt, bool depth);
};

} // namespace ToyFrameV
