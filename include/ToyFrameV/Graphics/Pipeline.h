#pragma once

/**
 * @file Pipeline.h
 * @brief Graphics pipeline state configuration
 */

#include "ToyFrameV/Graphics/Types.h"
#include <string>
#include <vector>

namespace ToyFrameV {

class Graphics;
class Shader;

/**
 * @brief Resource binding type for pipeline layout
 */
enum class ResourceBindingType { Texture, Sampler, ConstantBuffer };

/**
 * @brief Resource binding descriptor
 */
struct ResourceBinding {
  ResourceBindingType type = ResourceBindingType::Texture;
  uint32_t slot = 0; ///< Binding slot index
  std::string name;  ///< Resource name in shader (required for OpenGL)
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

    /// Resource bindings for textures, samplers, constant buffers
    std::vector<ResourceBinding> resourceBindings;
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
