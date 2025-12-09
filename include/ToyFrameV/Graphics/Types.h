#pragma once

/**
 * @file Types.h
 * @brief Basic graphics types: Color, Format, Topology, VertexLayout
 */

#include <string>
#include <vector>
#include <cstdint>

namespace ToyFrameV {

/**
 * @brief RGBA Color
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
 * @brief Pixel format for textures and render targets
 */
enum class PixelFormat {
    RGBA8,      // 8-bit per channel, 32-bit total
    RGB8,       // 8-bit per channel, 24-bit total (no alpha)
    BGRA8,      // 8-bit per channel, 32-bit total (BGR order)
    R8,         // Single channel, 8-bit
    RG8,        // Two channels, 16-bit total
    RGBA16F,    // 16-bit float per channel
    RGBA32F,    // 32-bit float per channel
    Depth24Stencil8,
    Depth32F
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
 * @brief Get the size in bytes for a vertex format
 */
uint32_t GetFormatSize(Format format);

/**
 * @brief Get bytes per pixel for a pixel format
 */
uint32_t GetBytesPerPixel(PixelFormat format);

} // namespace ToyFrameV
