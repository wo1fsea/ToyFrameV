/**
 * @file IGraphicsBackend.h
 * @brief Abstract graphics backend interface
 * 
 * This file defines the interface that all graphics backends must implement.
 * The Graphics class delegates all low-level operations to the backend,
 * allowing different rendering implementations (LLGL, direct Vulkan, etc.)
 */

#pragma once

#include "ToyFrameV/Graphics/Types.h"
#include "ToyFrameV/Window.h"
#include <memory>
#include <string>
#include <cstdint>
#include <functional>
#include <vector>

namespace ToyFrameV {

// Forward declarations
class Window;

/**
 * @brief Backend configuration
 */
struct BackendConfig {
    enum class API {
        Auto,
        Direct3D11,
        Direct3D12,
        OpenGL,
        Vulkan,
        Metal,
        WebGL
    };
    
    API api = API::Auto;
    bool vsync = true;
    int samples = 1;
    bool debugMode = false;
    uint32_t windowWidth = 800;
    uint32_t windowHeight = 600;
};

/**
 * @brief Opaque handle for backend resources
 * 
 * Backend implementations define their own resource types.
 * The frontend uses these handles without knowing the underlying type.
 */
using BackendHandle = void*;

/**
 * @brief Buffer creation descriptor
 */
struct BackendBufferDesc {
    enum class Type { Vertex, Index, Uniform };
    Type type = Type::Vertex;
    uint32_t size = 0;
    const void* initialData = nullptr;
    VertexLayout vertexLayout;  // Only for vertex buffers
};

/**
 * @brief Shader source descriptor
 */
struct BackendShaderSource {
    std::string code;
    std::string entryPoint = "main";
};

/**
 * @brief Shader creation descriptor
 */
struct BackendShaderDesc {
    BackendShaderSource vertexShader;
    BackendShaderSource fragmentShader;
    VertexLayout vertexLayout;
};

/**
 * @brief Resource binding descriptor for pipeline layout
 */
struct BackendResourceBinding {
  enum class Type { Texture, Sampler, ConstantBuffer };
  Type type = Type::Texture;
  uint32_t slot = 0; // Binding slot index
  std::string name;  // Resource name in shader (optional, for OpenGL)
};

/**
 * @brief Pipeline creation descriptor
 */
struct BackendPipelineDesc {
    BackendHandle vertexShader = nullptr;
    BackendHandle fragmentShader = nullptr;
    Topology topology = Topology::TriangleList;
    bool wireframe = false;
    bool cullBackFace = false;
    bool depthTestEnabled = true;
    bool depthWriteEnabled = true;
    bool blendEnabled = false;

    // Resource bindings for PipelineLayout
    std::vector<BackendResourceBinding> resourceBindings;
};

/**
 * @brief Render texture creation descriptor
 */
struct BackendRenderTextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    bool hasDepth = true;
};

/**
 * @brief Texture creation descriptor
 */
struct BackendTextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    bool generateMipmaps = false;
    const void* initialData = nullptr;
    size_t dataSize = 0;
};

/**
 * @brief Texture filter mode
 */
enum class BackendTextureFilter {
    Nearest,
    Linear
};

/**
 * @brief Texture address mode
 */
enum class BackendTextureAddressMode {
    Repeat,
    Mirror,
    Clamp,
    Border
};

/**
 * @brief Sampler creation descriptor
 */
struct BackendSamplerDesc {
    BackendTextureFilter minFilter = BackendTextureFilter::Linear;
    BackendTextureFilter magFilter = BackendTextureFilter::Linear;
    BackendTextureFilter mipFilter = BackendTextureFilter::Linear;
    BackendTextureAddressMode addressU = BackendTextureAddressMode::Repeat;
    BackendTextureAddressMode addressV = BackendTextureAddressMode::Repeat;
    BackendTextureAddressMode addressW = BackendTextureAddressMode::Repeat;
    float mipLODBias = 0.0f;
    uint32_t maxAnisotropy = 1;
    float borderColor[4] = {0, 0, 0, 0};
};

/**
 * @brief Pixel data container for readback operations
 */
struct BackendPixelData {
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGBA8;
    
    bool IsValid() const { return !data.empty() && width > 0 && height > 0; }
};

/**
 * @brief Readback completion callback
 */
using BackendReadbackCallback = std::function<void(BackendPixelData)>;

/**
 * @brief Abstract graphics backend interface
 * 
 * All graphics backends must implement this interface.
 * The Graphics class uses this interface to perform all rendering operations.
 */
class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;

    // ==================== Lifecycle ====================

    /**
     * @brief Initialize the backend
     * @param window External window to render to (may be nullptr)
     * @param config Backend configuration
     * @return true on success
     */
    virtual bool Initialize(Window* window, const BackendConfig& config) = 0;

    /**
     * @brief Shutdown and cleanup all resources
     */
    virtual void Shutdown() = 0;

    /**
     * @brief Process backend events
     * @return false if the backend requests shutdown
     */
    virtual bool ProcessEvents() = 0;

    /**
     * @brief Check if the backend is valid and ready
     */
    virtual bool IsValid() const = 0;

    // ==================== Frame Management ====================

    /**
     * @brief Begin a new frame
     */
    virtual void BeginFrame() = 0;

    /**
     * @brief End the current frame and present
     */
    virtual void EndFrame() = 0;

    /**
     * @brief Clear the current render target
     */
    virtual void Clear(const Color& color) = 0;

    /**
     * @brief Handle window resize
     */
    virtual void OnResize(uint32_t width, uint32_t height) = 0;

    // ==================== Resource Creation ====================

    /**
     * @brief Create a buffer (vertex, index, or uniform)
     * @return Handle to the created buffer, or nullptr on failure
     */
    virtual BackendHandle CreateBuffer(const BackendBufferDesc& desc) = 0;

    /**
     * @brief Destroy a buffer
     */
    virtual void DestroyBuffer(BackendHandle buffer) = 0;

    /**
     * @brief Create a shader program
     * @param desc Shader descriptor containing vertex and fragment shader sources
     * @param outVertexShader Output handle for vertex shader
     * @param outFragmentShader Output handle for fragment shader
     * @return true on success
     */
    virtual bool CreateShader(const BackendShaderDesc& desc, 
                              BackendHandle& outVertexShader, 
                              BackendHandle& outFragmentShader) = 0;

    /**
     * @brief Destroy shader resources
     */
    virtual void DestroyShader(BackendHandle vertexShader, BackendHandle fragmentShader) = 0;

    /**
     * @brief Create a graphics pipeline
     * @return Handle to the created pipeline, or nullptr on failure
     */
    virtual BackendHandle CreatePipeline(const BackendPipelineDesc& desc) = 0;

    /**
     * @brief Destroy a pipeline
     */
    virtual void DestroyPipeline(BackendHandle pipeline) = 0;

    /**
     * @brief Create a render texture (offscreen render target)
     * @return Handle to the created render texture, or nullptr on failure
     */
    virtual BackendHandle CreateRenderTexture(const BackendRenderTextureDesc& desc) = 0;

    /**
     * @brief Destroy a render texture
     */
    virtual void DestroyRenderTexture(BackendHandle renderTexture) = 0;

    /**
     * @brief Resize a render texture
     */
    virtual bool ResizeRenderTexture(BackendHandle renderTexture, uint32_t width, uint32_t height) = 0;

    /**
     * @brief Create a texture
     * @return Handle to the created texture, or nullptr on failure
     */
    virtual BackendHandle CreateTexture(const BackendTextureDesc& desc) = 0;

    /**
     * @brief Destroy a texture
     */
    virtual void DestroyTexture(BackendHandle texture) = 0;

    /**
     * @brief Create a sampler
     * @return Handle to the created sampler, or nullptr on failure
     */
    virtual BackendHandle CreateSampler(const BackendSamplerDesc& desc) = 0;

    /**
     * @brief Destroy a sampler
     */
    virtual void DestroySampler(BackendHandle sampler) = 0;

    // ==================== Render State ====================

    /**
     * @brief Set the current pipeline
     */
    virtual void SetPipeline(BackendHandle pipeline) = 0;

    /**
     * @brief Set the current vertex buffer
     */
    virtual void SetVertexBuffer(BackendHandle buffer) = 0;

    /**
     * @brief Set the current render target
     * @param renderTexture Render texture handle, or nullptr for screen
     */
    virtual void SetRenderTarget(BackendHandle renderTexture) = 0;

    /**
     * @brief Set a texture at the specified slot
     * @param slot Texture slot index (0-based)
     * @param texture Texture handle, or nullptr to unbind
     */
    virtual void SetTexture(uint32_t slot, BackendHandle texture) = 0;

    /**
     * @brief Set a sampler at the specified slot
     * @param slot Sampler slot index (0-based)
     * @param sampler Sampler handle, or nullptr to use default
     */
    virtual void SetSampler(uint32_t slot, BackendHandle sampler) = 0;

    // ==================== Drawing ====================

    /**
     * @brief Draw non-indexed primitives
     */
    virtual void Draw(uint32_t vertexCount, uint32_t firstVertex) = 0;

    /**
     * @brief Draw indexed primitives
     */
    virtual void DrawIndexed(uint32_t indexCount, uint32_t firstIndex) = 0;

    // ==================== Render Texture Operations ====================

    /**
     * @brief Read pixels from a render texture synchronously
     */
    virtual BackendPixelData ReadRenderTexturePixels(BackendHandle renderTexture) = 0;

    /**
     * @brief Read pixels from a render texture asynchronously
     */
    virtual void ReadRenderTexturePixelsAsync(BackendHandle renderTexture, BackendReadbackCallback callback) = 0;

    /**
     * @brief Check if async readback is pending
     */
    virtual bool IsReadbackPending(BackendHandle renderTexture) const = 0;

    /**
     * @brief Cancel pending async readback
     */
    virtual void CancelReadback(BackendHandle renderTexture) = 0;

    // ==================== Queries ====================

    /**
     * @brief Get the backend/renderer name
     */
    virtual const std::string& GetBackendName() const = 0;

    /**
     * @brief Get the device/GPU name
     */
    virtual const std::string& GetDeviceName() const = 0;

    /**
     * @brief Get the external window (if any)
     */
    virtual Window* GetWindow() const = 0;

    /**
     * @brief Check if the backend owns the window
     */
    virtual bool OwnsWindow() const = 0;
};

/**
 * @brief Create the default graphics backend (LLGL)
 */
std::unique_ptr<IGraphicsBackend> CreateDefaultBackend();

} // namespace ToyFrameV
