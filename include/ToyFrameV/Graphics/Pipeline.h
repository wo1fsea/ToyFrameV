#pragma once

/**
 * @file Pipeline.h
 * @brief Graphics pipeline state configuration
 */

#include "ToyFrameV/Graphics/Types.h"

namespace ToyFrameV {

class Graphics;
class Shader;

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
