#pragma once

/**
 * @file Shader.h
 * @brief Shader compilation and management
 */

#include "ToyFrameV/Graphics/Types.h"
#include <string>

namespace ToyFrameV {

class Graphics;

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

} // namespace ToyFrameV
