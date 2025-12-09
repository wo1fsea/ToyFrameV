#pragma once

/**
 * @file Context.h
 * @brief Graphics context - main rendering interface
 */

#include "ToyFrameV/Graphics/Types.h"
#include "ToyFrameV/Graphics/Buffer.h"
#include "ToyFrameV/Graphics/Shader.h"
#include "ToyFrameV/Graphics/Pipeline.h"
#include "ToyFrameV/Graphics/RenderTexture.h"
#include "ToyFrameV/Platform.h"
#include <memory>
#include <string>

namespace ToyFrameV {

class Window;
class GraphicsImpl;

/**
 * @brief Graphics backend type
 */
enum class GraphicsBackend {
    Auto,       // Auto-select best available
    Direct3D11,
    Direct3D12,
    OpenGL,
    Vulkan,
    Metal
};

/**
 * @brief Graphics configuration
 */
struct GraphicsConfig {
    GraphicsBackend backend = GraphicsBackend::Auto;
    bool vsync = true;
    int samples = 1;  // MSAA samples
    bool debugMode = false;
};

/**
 * @brief Graphics context - manages rendering system
 * 
 * This class provides a high-level abstraction over LLGL.
 * Users should never need to include LLGL headers directly.
 */
class Graphics {
public:
    ~Graphics();

    /**
     * @brief Initialize graphics system
     * @param window The window to render to (if nullptr, creates its own window)
     * @param config Graphics configuration
     * @return Graphics instance or nullptr on failure
     */
    static std::unique_ptr<Graphics> Create(Window* window = nullptr, const GraphicsConfig& config = GraphicsConfig{});

    // ==================== Frame Management ====================
    
    /**
     * @brief Begin a new frame
     */
    void BeginFrame();

    /**
     * @brief End the current frame and present
     */
    void EndFrame();

    /**
     * @brief Clear the render target
     */
    void Clear(const Color& color);

    // ==================== Resource Creation ====================

    /**
     * @brief Create a vertex buffer
     */
    std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& desc);

    /**
     * @brief Create a shader
     */
    std::unique_ptr<Shader> CreateShader(const ShaderDesc& desc);

    /**
     * @brief Create a graphics pipeline
     */
    std::unique_ptr<Pipeline> CreatePipeline(const PipelineDesc& desc);

    /**
     * @brief Create an offscreen render texture
     */
    std::unique_ptr<RenderTexture> CreateRenderTexture(const RenderTextureDesc& desc);

    // ==================== Render Target Management ====================

    /**
     * @brief Set current render target
     * @param rt RenderTexture to render to, or nullptr for screen
     */
    void SetRenderTarget(RenderTexture* rt);

    /**
     * @brief Get current render target
     * @return Current RenderTexture, or nullptr if rendering to screen
     */
    RenderTexture* GetRenderTarget() const;

    // ==================== Drawing Commands ====================

    /**
     * @brief Set the current pipeline
     */
    void SetPipeline(Pipeline* pipeline);

    /**
     * @brief Set vertex buffer
     */
    void SetVertexBuffer(Buffer* buffer);

    /**
     * @brief Draw primitives
     * @param vertexCount Number of vertices to draw
     * @param firstVertex First vertex index
     */
    void Draw(uint32_t vertexCount, uint32_t firstVertex = 0);

    /**
     * @brief Draw indexed primitives
     */
    void DrawIndexed(uint32_t indexCount, uint32_t firstIndex = 0);

    // ==================== Async Operations ====================

    /**
     * @brief Process pending async readback callbacks
     * @note Call this once per frame for async RenderTexture readback
     */
    void ProcessReadbacks();

    // ==================== Queries ====================

    /**
     * @brief Get current backend name
     */
    const std::string& GetBackendName() const;

    /**
     * @brief Get device name (GPU)
     */
    const std::string& GetDeviceName() const;

    /**
     * @brief Get window associated with this graphics context
     */
    Window* GetWindow() const;

    /**
     * @brief Handle window resize
     */
    void OnResize(int width, int height);

    /**
     * @brief Process window events (call each frame)
     * @return false if window should close
     * @note When using an external window, this only checks if the swap chain
     * is valid. Window events are handled by WindowSystem.
     */
    bool ProcessEvents();

    /**
     * @brief Check if graphics context is valid and ready for rendering
     */
    bool IsValid() const;

    // Internal - get implementation
    GraphicsImpl* GetImpl() const { return m_impl.get(); }

    // Internal - get backend interface
    class IGraphicsBackend *GetBackend() const;

  private:
    Graphics() = default;
    bool Initialize(Window* window, const GraphicsConfig& config);

    std::unique_ptr<GraphicsImpl> m_impl;
    std::string m_backendName;
    std::string m_deviceName;
    RenderTexture* m_currentRenderTarget = nullptr;
};

} // namespace ToyFrameV
