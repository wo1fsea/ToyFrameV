#pragma once

/**
 * @file Buffer.h
 * @brief GPU Buffer classes for vertex, index, and uniform data
 */

#include "ToyFrameV/Graphics/Types.h"
#include <memory>

namespace ToyFrameV {

class Graphics;

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

} // namespace ToyFrameV
