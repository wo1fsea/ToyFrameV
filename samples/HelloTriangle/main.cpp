/**
 * @file main.cpp
 * @brief HelloTriangle - Stage 4 verification sample
 * 
 * This sample demonstrates how to render a colored triangle using ToyFrameV.
 * Note: This code ONLY uses ToyFrameV API - no LLGL headers required!
 * 
 * Supports: Direct3D11 (Windows), OpenGL (Desktop), WebGL (Browser)
 */

#include <ToyFrameV.h>
#include <iostream>

using namespace ToyFrameV;

// ============================================================================
// Shader Source Code - HLSL (Direct3D)
// ============================================================================

const char* g_vertexShaderHLSL = R"(
struct VertexInput {
    float2 position : POSITION;
    float4 color : COLOR;
};

struct VertexOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

VertexOutput VS(VertexInput input) {
    VertexOutput output;
    output.position = float4(input.position, 0.0, 1.0);
    output.color = input.color;
    return output;
}
)";

const char* g_fragmentShaderHLSL = R"(
struct VertexOutput {
    float4 position : SV_Position;
    float4 color : COLOR;
};

float4 PS(VertexOutput input) : SV_Target {
    return input.color;
}
)";

// ============================================================================
// Shader Source Code - GLSL (OpenGL / WebGL)
// ============================================================================

const char* g_vertexShaderGLSL = R"(#version 300 es
precision mediump float;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

out vec4 vColor;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = color;
}
)";

const char* g_fragmentShaderGLSL = R"(#version 300 es
precision mediump float;

in vec4 vColor;
out vec4 fragColor;

void main() {
    fragColor = vColor;
}
)";

// ============================================================================
// Vertex Data
// ============================================================================

struct Vertex {
    float position[2];
    uint8_t color[4];
};

// ============================================================================
// HelloTriangle Application
// ============================================================================

class HelloTriangleApp : public App {
public:
    HelloTriangleApp() {
        m_config.title = "Hello Triangle";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
        m_config.graphics.vsync = true;
    }

protected:
    bool OnInit() override {
        auto* gfx = GetGraphics();
        
        std::cout << "Backend: " << gfx->GetBackendName() << std::endl;
        std::cout << "Device: " << gfx->GetDeviceName() << std::endl;

        // Create vertex layout
        VertexLayout layout;
        layout.Add("POSITION", Format::Float2)
              .Add("COLOR", Format::UByte4Norm);

        // Create vertex buffer
        const float s = 0.5f;
        Vertex vertices[] = {
            { {  0,  s }, { 255, 0, 0, 255 } },   // Top (red)
            { {  s, -s }, { 0, 255, 0, 255 } },   // Bottom right (green)
            { { -s, -s }, { 0, 0, 255, 255 } },   // Bottom left (blue)
        };

        BufferDesc bufferDesc;
        bufferDesc.type = BufferType::Vertex;
        bufferDesc.size = sizeof(vertices);
        bufferDesc.initialData = vertices;
        bufferDesc.vertexLayout = layout;

        m_vertexBuffer = gfx->CreateBuffer(bufferDesc);
        if (!m_vertexBuffer) {
            std::cerr << "Failed to create vertex buffer" << std::endl;
            return false;
        }

        // Create shader - select based on backend
        ShaderDesc shaderDesc;
        shaderDesc.vertexLayout = layout;
        
        // Check if using OpenGL/WebGL (GLSL) or Direct3D (HLSL)
        bool useGLSL = (gfx->GetBackendName().find("OpenGL") != std::string::npos) ||
                       (gfx->GetBackendName().find("WebGL") != std::string::npos);
        
        if (useGLSL) {
            shaderDesc.vertexShader = { ShaderStage::Vertex, g_vertexShaderGLSL, "main" };
            shaderDesc.fragmentShader = { ShaderStage::Fragment, g_fragmentShaderGLSL, "main" };
        } else {
            shaderDesc.vertexShader = { ShaderStage::Vertex, g_vertexShaderHLSL, "VS" };
            shaderDesc.fragmentShader = { ShaderStage::Fragment, g_fragmentShaderHLSL, "PS" };
        }

        m_shader = gfx->CreateShader(shaderDesc);
        if (!m_shader) {
            std::cerr << "Failed to create shader" << std::endl;
            return false;
        }

        // Create pipeline
        PipelineDesc pipelineDesc;
        pipelineDesc.shader = m_shader.get();
        pipelineDesc.topology = Topology::TriangleList;

        m_pipeline = gfx->CreatePipeline(pipelineDesc);
        if (!m_pipeline) {
            std::cerr << "Failed to create pipeline" << std::endl;
            return false;
        }

        std::cout << "Hello Triangle initialized!" << std::endl;
        return true;
    }

    void OnRender() override {
        auto* gfx = GetGraphics();

        // Clear to dark blue
        gfx->Clear(Color(0.1f, 0.1f, 0.2f));

        // Draw triangle
        gfx->SetPipeline(m_pipeline.get());
        gfx->SetVertexBuffer(m_vertexBuffer.get());
        gfx->Draw(3);
    }

    void OnShutdown() override {
        std::cout << "Hello Triangle shutdown!" << std::endl;
        m_pipeline.reset();
        m_shader.reset();
        m_vertexBuffer.reset();
    }

private:
    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Pipeline> m_pipeline;
};

// ============================================================================
// Entry Point
// ============================================================================

TOYFRAMEV_MAIN(HelloTriangleApp)
