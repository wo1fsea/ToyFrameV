/**
 * @file main.cpp
 * @brief HelloRenderTexture - Demonstrates offscreen rendering and BMP export
 *
 * This sample shows how to:
 * - Create a RenderTexture for offscreen rendering
 * - Render a colored triangle to the texture
 * - Read back pixel data and save to BMP file
 * - Use TimerSystem to schedule automatic screenshots
 * - On WebGL: Download all screenshots as a ZIP file
 */

#include <ToyFrameV.h>
#include <cmath>
#include <iostream>

using namespace ToyFrameV;
using namespace ToyFrameV::Core;

// ============================================================================
// Shader Source Code - HLSL (Direct3D)
// ============================================================================

const char *g_vertexShaderHLSL = R"(
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

const char *g_fragmentShaderHLSL = R"(
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

const char *g_vertexShaderGLSL = R"(#version 300 es
precision mediump float;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;

out vec4 vColor;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vColor = color;
}
)";

const char *g_fragmentShaderGLSL = R"(#version 300 es
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

class HelloRenderTextureApp : public App {
public:
  HelloRenderTextureApp() {
    m_config.title = "Hello RenderTexture";
    m_config.windowWidth = 800;
    m_config.windowHeight = 600;
  }

protected:
  bool OnInit() override {
    std::cout << "========================================" << std::endl;
    std::cout << "    HelloRenderTexture Demo" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Backend: " << GetGraphics()->GetBackendName() << std::endl;
    std::cout << "Device: " << GetGraphics()->GetDeviceName() << std::endl;
    std::cout << "========================================" << std::endl;

    auto *gfx = GetGraphics();

    // Create vertex layout
    VertexLayout layout;
    layout.Add("POSITION", Format::Float2).Add("COLOR", Format::UByte4Norm);

    // Create vertex buffer - triangle vertices
    const float s = 0.6f;
    Vertex vertices[] = {
        {{0, s}, {255, 0, 0, 255}},   // Top (red)
        {{s, -s}, {0, 255, 0, 255}},  // Bottom right (green)
        {{-s, -s}, {0, 0, 255, 255}}, // Bottom left (blue)
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

    // Create an offscreen render texture (256x256)
    RenderTextureDesc rtDesc;
    rtDesc.width = 256;
    rtDesc.height = 256;
    rtDesc.format = PixelFormat::RGBA8;
    rtDesc.hasDepth = true;

    m_renderTexture = gfx->CreateRenderTexture(rtDesc);
    if (!m_renderTexture) {
      TOYFRAMEV_LOG_ERROR("Failed to create RenderTexture!");
      return false;
    }

    TOYFRAMEV_LOG_INFO("RenderTexture created: {}x{}",
                       m_renderTexture->GetWidth(),
                       m_renderTexture->GetHeight());

    // Use TimerSystem to take screenshots every 2 seconds
    auto *timerSystem = GetSystem<TimerSystem>();
    if (timerSystem) {
      // Take screenshots at 1s, 3s, 5s, 7s, 9s
      const int numScreenshots = 5;
      for (int i = 0; i < numScreenshots; ++i) {
        double time = 1.0 + i * 2.0; // 1, 3, 5, 7, 9 seconds
        int index = i + 1;

        timerSystem->SetTimeout(time, [this, index, numScreenshots]() {
          // Request screenshot - will be taken after next render
          m_screenshotFilename = "screenshot_" + std::to_string(index) + ".bmp";
          m_screenshotIndex = index;
          m_totalScreenshots = numScreenshots;
          m_screenshotRequested = true;
        });
      }

      TOYFRAMEV_LOG_INFO("Scheduled {} screenshots", numScreenshots);
    }

    return true;
  }

  void OnUpdate(float deltaTime) override {
    m_time += deltaTime;

    // Screenshot state machine:
    // Frame N: Timer fires -> m_screenshotRequested = true
    // Frame N: OnUpdate -> detect request, set m_waitFrames = 2
    // Frame N: OnRender -> render triangle
    // Frame N+1: OnUpdate -> m_waitFrames = 1
    // Frame N+1: OnRender -> render triangle
    // Frame N+2: OnUpdate -> m_waitFrames = 0, set m_pendingScreenshot = true
    // Frame N+2: OnRender -> render, then read pixels (now safe)

    if (m_screenshotRequested) {
      // Start waiting for frames to ensure GPU has finished
      m_waitFrames = 2; // Wait 2 frames for WebGL safety
      m_screenshotRequested = false;
    }

    if (m_waitFrames > 0) {
      m_waitFrames--;
      if (m_waitFrames == 0) {
        m_pendingScreenshot = true;
      }
    }
  }

  void OnRender() override {
    auto *gfx = GetGraphics();

    // First, render triangle to the RenderTexture
    gfx->SetRenderTarget(m_renderTexture.get());
    {
      // Clear to animated background color
      float r = (std::sin(m_time * 2.0f) + 1.0f) * 0.15f;
      float g = (std::sin(m_time * 3.0f) + 1.0f) * 0.15f;
      float b = (std::sin(m_time * 1.5f) + 1.0f) * 0.15f + 0.1f;
      gfx->Clear(Color(r, g, b));

      // Draw colored triangle to RenderTexture
      gfx->SetPipeline(m_pipeline.get());
      gfx->SetVertexBuffer(m_vertexBuffer.get());
      gfx->Draw(3);
    }

    // Switch back to screen rendering
    gfx->SetRenderTarget(nullptr);
    {
      // Clear screen with a different color
      gfx->Clear(Color::CornflowerBlue());

      // Draw the same triangle on screen for comparison
      gfx->SetPipeline(m_pipeline.get());
      gfx->SetVertexBuffer(m_vertexBuffer.get());
      gfx->Draw(3);
    }

    // Take screenshot after rendering is complete (delayed by 2 frames for
    // WebGL)
    if (m_pendingScreenshot) {
      m_pendingScreenshot = false;
      DoTakeScreenshot();
    }
  }

  void OnShutdown() override {
    m_renderTexture.reset();
    m_pipeline.reset();
    m_shader.reset();
    m_vertexBuffer.reset();
    std::cout << "HelloRenderTexture shutdown!" << std::endl;
  }

private:
  void DoTakeScreenshot() {
    if (!m_renderTexture || m_screenshotFilename.empty())
      return;

    TOYFRAMEV_LOG_INFO("Taking screenshot: {}", m_screenshotFilename);

    // Read pixels from RenderTexture (synchronous)
    PixelData pixels = m_renderTexture->ReadPixels();

    if (pixels.IsValid()) {
      // Debug: Check some pixel values to verify content
      size_t centerIdx =
          (pixels.height / 2 * pixels.width + pixels.width / 2) * 4;
      if (pixels.data.size() > centerIdx + 3) {
        TOYFRAMEV_LOG_INFO(
            "Center pixel RGBA: ({}, {}, {}, {})", (int)pixels.data[centerIdx],
            (int)pixels.data[centerIdx + 1], (int)pixels.data[centerIdx + 2],
            (int)pixels.data[centerIdx + 3]);
      }

      if (pixels.SaveToBMP(m_screenshotFilename)) {
        TOYFRAMEV_LOG_INFO("Screenshot queued/saved: {} ({}x{})",
                           m_screenshotFilename, pixels.width, pixels.height);
      } else {
        TOYFRAMEV_LOG_ERROR("Failed to save screenshot: {}",
                            m_screenshotFilename);
      }
    } else {
      TOYFRAMEV_LOG_ERROR("Failed to read pixels from RenderTexture");
    }

    // After last screenshot, download ZIP (WebGL) or quit
    if (m_screenshotIndex == m_totalScreenshots) {
      FinishDemo();
    }

    m_screenshotFilename.clear();
  }

  void FinishDemo() {
    TOYFRAMEV_LOG_INFO("Demo complete!");

    // Platform-agnostic handling of screenshots
    size_t count = PixelData::GetPendingCount();
    if (count > 0) {
      TOYFRAMEV_LOG_INFO("Downloading {} screenshots as ZIP...", count);
      PixelData::DownloadAllAsZip("screenshots.zip");
    } else {
      TOYFRAMEV_LOG_INFO("Screenshots saved to current directory.");
    }

    // Request quit after a short delay to allow download to trigger
    auto *timerSystem = GetSystem<TimerSystem>();
    if (timerSystem) {
      timerSystem->SetTimeout(0.5, [this]() { Quit(); });
    } else {
      Quit();
    }
  }

  std::unique_ptr<RenderTexture> m_renderTexture;
  std::unique_ptr<Buffer> m_vertexBuffer;
  std::unique_ptr<Shader> m_shader;
  std::unique_ptr<Pipeline> m_pipeline;
  float m_time = 0.0f;

  // Screenshot state
  bool m_screenshotRequested = false;
  bool m_pendingScreenshot = false;
  int m_waitFrames = 0;
  std::string m_screenshotFilename;
  int m_screenshotIndex = 0;
  int m_totalScreenshots = 0;
};

// Entry point
TOYFRAMEV_MAIN(HelloRenderTextureApp)
