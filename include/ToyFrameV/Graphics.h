#pragma once

#include "ToyFrameV/Platform.h"
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <functional>

namespace ToyFrameV {

// Forward declarations
class Window;
class GraphicsImpl;
class Buffer;
class Shader;
class Pipeline;

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
 * @brief Clear color
 */
struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;

    Color() = default;
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
    
    static Color Black() { return Color(0, 0, 0, 1); }
    static Color White() { return Color(1, 1, 1, 1); }
    static Color Red() { return Color(1, 0, 0, 1); }
    static Color Green() { return Color(0, 1, 0, 1); }
    static Color Blue() { return Color(0, 0, 1, 1); }
    static Color CornflowerBlue() { return Color(0.392f, 0.584f, 0.929f, 1); }
};

/**
 * @brief Vertex attribute format
 */
enum class Format {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UByte4Norm,   // Normalized 4 bytes -> 4 floats [0,1]
    UByte4        // 4 bytes as uint
};

/**
 * @brief Vertex attribute description
 */
struct VertexAttribute {
    std::string name;
    Format format;
    uint32_t offset = 0;

    VertexAttribute() = default;
    VertexAttribute(const std::string& name, Format format, uint32_t offset = 0)
        : name(name), format(format), offset(offset) {}
};

/**
 * @brief Vertex layout description
 */
struct VertexLayout {
    std::vector<VertexAttribute> attributes;
    uint32_t stride = 0;

    VertexLayout& Add(const std::string& name, Format format);
    void CalculateOffsetsAndStride();
};

/**
 * @brief Buffer type
 */
enum class BufferType {
    Vertex,
    Index,
    Uniform
};

/**
 * @brief Buffer description
 */
struct BufferDesc {
    BufferType type = BufferType::Vertex;
    uint32_t size = 0;
    const void* initialData = nullptr;
    VertexLayout vertexLayout;  // For vertex buffers
};

/**
 * @brief Shader stage
 */
enum class ShaderStage {
    Vertex,
    Fragment,
    Compute
};

/**
 * @brief Shader source
 */
struct ShaderSource {
    ShaderStage stage;
    std::string code;         // Source code
    std::string entryPoint;   // Entry function name (e.g., "VS", "PS", "main")

    ShaderSource() = default;
    ShaderSource(ShaderStage stage, const std::string& code, const std::string& entry = "main")
        : stage(stage), code(code), entryPoint(entry) {}
};

/**
 * @brief Shader description
 */
struct ShaderDesc {
    ShaderSource vertexShader;
    ShaderSource fragmentShader;
    VertexLayout vertexLayout;
};

/**
 * @brief Primitive topology
 */
enum class Topology {
    TriangleList,
    TriangleStrip,
    LineList,
    LineStrip,
    PointList
};

/**
 * @brief Pipeline description
 */
struct PipelineDesc {
    Shader* shader = nullptr;
    Topology topology = Topology::TriangleList;
    bool wireframe = false;
    bool cullBackFace = false;
    bool depthTestEnabled = false;
    bool depthWriteEnabled = false;
    bool blendEnabled = false;
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
     */
    bool ProcessEvents();

    // Internal - get implementation
    GraphicsImpl* GetImpl() const { return m_impl.get(); }

private:
    Graphics() = default;
    bool Initialize(Window* window, const GraphicsConfig& config);

    std::unique_ptr<GraphicsImpl> m_impl;
    std::string m_backendName;
    std::string m_deviceName;
};

/**
 * @brief GPU Buffer
 */
class Buffer {
public:
    ~Buffer();
    
    BufferType GetType() const { return m_type; }
    uint32_t GetSize() const { return m_size; }

    // Internal
    void* GetHandle() const { return m_handle; }

private:
    friend class Graphics;
    Buffer() = default;

    void* m_handle = nullptr;
    BufferType m_type = BufferType::Vertex;
    uint32_t m_size = 0;
    Graphics* m_graphics = nullptr;
};

/**
 * @brief Compiled Shader
 */
class Shader {
public:
    ~Shader();

    // Internal
    void* GetVertexHandle() const { return m_vertexHandle; }
    void* GetFragmentHandle() const { return m_fragmentHandle; }

private:
    friend class Graphics;
    Shader() = default;

    void* m_vertexHandle = nullptr;
    void* m_fragmentHandle = nullptr;
    Graphics* m_graphics = nullptr;
};

/**
 * @brief Graphics Pipeline
 */
class Pipeline {
public:
    ~Pipeline();

    // Internal
    void* GetHandle() const { return m_handle; }

private:
    friend class Graphics;
    Pipeline() = default;

    void* m_handle = nullptr;
    Graphics* m_graphics = nullptr;
};

} // namespace ToyFrameV
