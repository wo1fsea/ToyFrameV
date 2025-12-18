/**
 * @file LLGLTypes.h
 * @brief LLGL type conversion utilities
 * 
 * Converts ToyFrameV types to LLGL types.
 */

#pragma once

#include "ToyFrameV/Graphics/Types.h"
#include <LLGL/LLGL.h>

namespace ToyFrameV {

/**
 * @brief Convert ToyFrameV Format to LLGL Format
 */
inline LLGL::Format ToLLGLFormat(Format format) {
    switch (format) {
        case Format::Float:      return LLGL::Format::R32Float;
        case Format::Float2:     return LLGL::Format::RG32Float;
        case Format::Float3:     return LLGL::Format::RGB32Float;
        case Format::Float4:     return LLGL::Format::RGBA32Float;
        case Format::Int:        return LLGL::Format::R32SInt;
        case Format::Int2:       return LLGL::Format::RG32SInt;
        case Format::Int3:       return LLGL::Format::RGB32SInt;
        case Format::Int4:       return LLGL::Format::RGBA32SInt;
        case Format::UByte4Norm: return LLGL::Format::RGBA8UNorm;
        case Format::UByte4:     return LLGL::Format::RGBA8UInt;
        default:                 return LLGL::Format::Undefined;
    }
}

/**
 * @brief Convert ToyFrameV PixelFormat to LLGL Format
 */
inline LLGL::Format ToLLGLPixelFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA8:            return LLGL::Format::RGBA8UNorm;
        case PixelFormat::BGRA8:            return LLGL::Format::BGRA8UNorm;
        case PixelFormat::RGB8:             return LLGL::Format::RGB8UNorm;
        case PixelFormat::R8:               return LLGL::Format::R8UNorm;
        case PixelFormat::RG8:              return LLGL::Format::RG8UNorm;
        case PixelFormat::RGBA16F:          return LLGL::Format::RGBA16Float;
        case PixelFormat::RGBA32F:          return LLGL::Format::RGBA32Float;
        case PixelFormat::Depth24Stencil8:  return LLGL::Format::D24UNormS8UInt;
        case PixelFormat::Depth32F:         return LLGL::Format::D32Float;
        default:                            return LLGL::Format::RGBA8UNorm;
    }
}

/**
 * @brief Convert ToyFrameV PixelFormat to LLGL ImageFormat for texture upload
 */
inline LLGL::ImageFormat ToLLGLImageFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA8:            return LLGL::ImageFormat::RGBA;
        case PixelFormat::BGRA8:            return LLGL::ImageFormat::BGRA;
        case PixelFormat::RGB8:             return LLGL::ImageFormat::RGB;
        case PixelFormat::R8:               return LLGL::ImageFormat::R;
        case PixelFormat::RG8:              return LLGL::ImageFormat::RG;
        case PixelFormat::RGBA16F:          return LLGL::ImageFormat::RGBA;
        case PixelFormat::RGBA32F:          return LLGL::ImageFormat::RGBA;
        default:                            return LLGL::ImageFormat::RGBA;
    }
}

/**
 * @brief Get LLGL DataType for a PixelFormat
 */
inline LLGL::DataType ToLLGLDataType(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGBA8:
        case PixelFormat::BGRA8:
        case PixelFormat::RGB8:
        case PixelFormat::R8:
        case PixelFormat::RG8:
            return LLGL::DataType::UInt8;
        case PixelFormat::RGBA16F:
            return LLGL::DataType::Float16;
        case PixelFormat::RGBA32F:
            return LLGL::DataType::Float32;
        default:
            return LLGL::DataType::UInt8;
    }
}

/**
 * @brief Convert ToyFrameV Topology to LLGL PrimitiveTopology
 */
inline LLGL::PrimitiveTopology ToLLGLTopology(Topology topology) {
    switch (topology) {
        case Topology::TriangleList:  return LLGL::PrimitiveTopology::TriangleList;
        case Topology::TriangleStrip: return LLGL::PrimitiveTopology::TriangleStrip;
        case Topology::LineList:      return LLGL::PrimitiveTopology::LineList;
        case Topology::LineStrip:     return LLGL::PrimitiveTopology::LineStrip;
        case Topology::PointList:     return LLGL::PrimitiveTopology::PointList;
        default:                      return LLGL::PrimitiveTopology::TriangleList;
    }
}

/**
 * @brief Get the API module name string for LLGL
 */
inline const char* GetLLGLModuleName(BackendConfig::API api) {
    switch (api) {
        case BackendConfig::API::Direct3D11: return "Direct3D11";
        case BackendConfig::API::Direct3D12: return "Direct3D12";
        case BackendConfig::API::OpenGL:     return "OpenGL";
        case BackendConfig::API::Vulkan:     return "Vulkan";
        case BackendConfig::API::Metal:      return "Metal";
        case BackendConfig::API::WebGL:      return "WebGL";
        default:                             return nullptr;  // Auto-select
    }
}

/**
 * @brief Convert BackendTextureFilter to LLGL SamplerFilter
 */
inline LLGL::SamplerFilter ToLLGLFilter(BackendTextureFilter filter) {
    switch (filter) {
        case BackendTextureFilter::Nearest: return LLGL::SamplerFilter::Nearest;
        case BackendTextureFilter::Linear:  return LLGL::SamplerFilter::Linear;
        default:                            return LLGL::SamplerFilter::Linear;
    }
}

/**
 * @brief Convert BackendTextureAddressMode to LLGL SamplerAddressMode
 */
inline LLGL::SamplerAddressMode ToLLGLAddressMode(BackendTextureAddressMode mode) {
    switch (mode) {
        case BackendTextureAddressMode::Repeat:  return LLGL::SamplerAddressMode::Repeat;
        case BackendTextureAddressMode::Mirror:  return LLGL::SamplerAddressMode::Mirror;
        case BackendTextureAddressMode::Clamp:   return LLGL::SamplerAddressMode::Clamp;
        case BackendTextureAddressMode::Border:  return LLGL::SamplerAddressMode::Border;
        default:                                 return LLGL::SamplerAddressMode::Repeat;
    }
}

} // namespace ToyFrameV
