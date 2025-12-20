/**
 * @file main.cpp
 * @brief HelloTexture - Demonstrates texture loading and rendering
 *
 * This sample shows how to:
 * - Load textures using AssetSystem
 * - Create textures from procedural data
 * - Render textured quads with shaders
 * - Use texture samplers for filtering
 */

#include <ToyFrameV.h>
#include <cmath>
#include <iostream>

using namespace ToyFrameV;
using namespace ToyFrameV::Core;

// ============================================================================
// Shader Source Code - HLSL (Direct3D)
// ============================================================================

const char* g_vertexShaderHLSL = R"(
struct VertexInput {
    float2 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

VertexOutput VS(VertexInput input) {
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.texCoord = input.texCoord;
    return output;
}
)";

const char* g_fragmentShaderHLSL = R"(
Texture2D colorTexture : register(t0);
SamplerState colorSampler : register(s0);

struct VertexOutput {
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

float4 PS(VertexOutput input) : SV_Target {
    return colorTexture.Sample(colorSampler, input.texCoord);
}
)";

// ============================================================================
// Shader Source Code - GLSL (OpenGL / WebGL)
// ============================================================================

const char* g_vertexShaderGLSL = R"(#version 300 es
precision mediump float;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vTexCoord = texCoord;
}
)";

const char* g_fragmentShaderGLSL = R"(#version 300 es
precision mediump float;

uniform sampler2D colorTexture;

in vec2 vTexCoord;
out vec4 fragColor;

void main() {
    fragColor = texture(colorTexture, vTexCoord);
}
)";

// ============================================================================
// Vertex Data
// ============================================================================

struct Vertex {
    float position[2];
    float texCoord[2];
};

// ============================================================================
// HelloTexture Application
// ============================================================================

class HelloTextureApp : public App {
  public:
    HelloTextureApp() {
        m_config.title = "Hello Texture";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
        m_config.graphics.vsync = true;
    }

  protected:
    bool OnInit() override {
        std::cout << "========================================" << std::endl;
        std::cout << "    HelloTexture Demo" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Backend: " << GetGraphics()->GetBackendName() << std::endl;
        std::cout << "Device: " << GetGraphics()->GetDeviceName() << std::endl;
        std::cout << "========================================" << std::endl;

        auto* gfx = GetGraphics();

        // Create vertex layout
        VertexLayout layout;
        layout.Add("POSITION", Format::Float2).Add("TEXCOORD", Format::Float2);

        // Create vertex buffer - quad vertices
        Vertex vertices[] = {
            // First triangle
            {{-0.5f, 0.5f}, {0.0f, 0.0f}},   // Top-left
            {{0.5f, 0.5f}, {1.0f, 0.0f}},    // Top-right
            {{0.5f, -0.5f}, {1.0f, 1.0f}},   // Bottom-right
            // Second triangle
            {{-0.5f, 0.5f}, {0.0f, 0.0f}},   // Top-left
            {{0.5f, -0.5f}, {1.0f, 1.0f}},   // Bottom-right
            {{-0.5f, -0.5f}, {0.0f, 1.0f}},  // Bottom-left
        };

        BufferDesc bufferDesc;
        bufferDesc.type = BufferType::Vertex;
        bufferDesc.size = sizeof(vertices);
        bufferDesc.initialData = vertices;
        bufferDesc.vertexLayout = layout;

        m_vertexBuffer = gfx->CreateBuffer(bufferDesc);
        if (!m_vertexBuffer) {
            TOYFRAMEV_LOG_ERROR("Failed to create vertex buffer");
            return false;
        }

        // Create shader - select based on backend
        ShaderDesc shaderDesc;
        shaderDesc.vertexLayout = layout;

        bool useGLSL =
            (gfx->GetBackendName().find("OpenGL") != std::string::npos) ||
            (gfx->GetBackendName().find("WebGL") != std::string::npos);

        if (useGLSL) {
            shaderDesc.vertexShader = {ShaderStage::Vertex, g_vertexShaderGLSL,
                                       "main"};
            shaderDesc.fragmentShader = {ShaderStage::Fragment, g_fragmentShaderGLSL,
                                         "main"};
        } else {
            shaderDesc.vertexShader = {ShaderStage::Vertex, g_vertexShaderHLSL, "VS"};
            shaderDesc.fragmentShader = {ShaderStage::Fragment, g_fragmentShaderHLSL,
                                         "PS"};
        }

        m_shader = gfx->CreateShader(shaderDesc);
        if (!m_shader) {
            TOYFRAMEV_LOG_ERROR("Failed to create shader");
            return false;
        }

        // Create pipeline
        PipelineDesc pipelineDesc;
        pipelineDesc.shader = m_shader.get();
        pipelineDesc.topology = Topology::TriangleList;

        m_pipeline = gfx->CreatePipeline(pipelineDesc);
        if (!m_pipeline) {
            TOYFRAMEV_LOG_ERROR("Failed to create pipeline");
            return false;
        }

        // Create a procedural checkerboard texture
        if (!CreateCheckerboardTexture()) {
            TOYFRAMEV_LOG_ERROR("Failed to create checkerboard texture");
            return false;
        }

        // Create sampler
        SamplerDesc samplerDesc;
        samplerDesc.minFilter = TextureFilter::Linear;
        samplerDesc.magFilter = TextureFilter::Linear;
        samplerDesc.addressU = TextureAddressMode::Repeat;
        samplerDesc.addressV = TextureAddressMode::Repeat;

        m_sampler = gfx->CreateSampler(samplerDesc);
        if (!m_sampler) {
            TOYFRAMEV_LOG_ERROR("Failed to create sampler");
            return false;
        }

        TOYFRAMEV_LOG_INFO("HelloTexture initialized successfully!");
        return true;
    }

    void OnUpdate(float deltaTime) override {
        m_time += deltaTime;
    }

    void OnRender() override {
        auto* gfx = GetGraphics();

        // Clear to dark blue
        gfx->Clear(Color(0.1f, 0.1f, 0.2f));

        // Set pipeline
        gfx->SetPipeline(m_pipeline.get());
        gfx->SetVertexBuffer(m_vertexBuffer.get());

        // Set texture and sampler
        gfx->SetTexture(0, m_texture.get());
        gfx->SetSampler(0, m_sampler.get());

        // Draw quad (6 vertices, 2 triangles)
        gfx->Draw(6);
    }

    void OnShutdown() override {
        std::cout << "HelloTexture shutdown!" << std::endl;
        m_sampler.reset();
        m_texture.reset();
        m_pipeline.reset();
        m_shader.reset();
        m_vertexBuffer.reset();
    }

  private:
    bool CreateCheckerboardTexture() {
        const uint32_t width = 64;
        const uint32_t height = 64;
        const uint32_t checkerSize = 8;

        // Generate checkerboard pattern
        std::vector<uint8_t> pixels(width * height * 4);

        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint32_t idx = (y * width + x) * 4;
                bool isWhite = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;

                if (isWhite) {
                    pixels[idx + 0] = 255;  // R
                    pixels[idx + 1] = 255;  // G
                    pixels[idx + 2] = 255;  // B
                    pixels[idx + 3] = 255;  // A
                } else {
                    pixels[idx + 0] = 50;   // R
                    pixels[idx + 1] = 50;   // G
                    pixels[idx + 2] = 200;  // B (dark blue)
                    pixels[idx + 3] = 255;  // A
                }
            }
        }

        // Create texture
        TextureDesc texDesc;
        texDesc.width = width;
        texDesc.height = height;
        texDesc.format = PixelFormat::RGBA8;
        texDesc.generateMipmaps = false;
        texDesc.initialData = pixels.data();
        texDesc.dataSize = pixels.size();

        m_texture = GetGraphics()->CreateTexture(texDesc);
        return m_texture != nullptr;
    }

    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Pipeline> m_pipeline;
    std::unique_ptr<Texture> m_texture;
    std::unique_ptr<Sampler> m_sampler;
    float m_time = 0.0f;
};

// Entry point
TOYFRAMEV_MAIN(HelloTextureApp)
