/**
 * @file main.cpp
 * @brief HelloRenderTexture - Demonstrates offscreen rendering and BMP export
 * 
 * This sample shows how to:
 * - Create a RenderTexture for offscreen rendering
 * - Render to the texture instead of screen
 * - Read back pixel data and save to BMP file
 * - Use TimerSystem to schedule automatic screenshots
 */

#include <ToyFrameV.h>
#include <iostream>
#include <cmath>

using namespace ToyFrameV;
using namespace ToyFrameV::Core;

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

        // Create an offscreen render texture (256x256)
        RenderTextureDesc rtDesc;
        rtDesc.width = 256;
        rtDesc.height = 256;
        rtDesc.format = PixelFormat::RGBA8;
        rtDesc.hasDepth = true;

        m_renderTexture = GetGraphics()->CreateRenderTexture(rtDesc);
        if (!m_renderTexture) {
            TOYFRAMEV_LOG_ERROR("Failed to create RenderTexture!");
            return false;
        }

        TOYFRAMEV_LOG_INFO("RenderTexture created: {}x{}", 
            m_renderTexture->GetWidth(), m_renderTexture->GetHeight());

        // Use TimerSystem to take screenshots every 2 seconds
        auto* timerSystem = GetSystem<TimerSystem>();
        if (timerSystem) {
            // Take first screenshot after 1 second
            timerSystem->SetTimeout(1.0, [this]() {
                TakeScreenshot("screenshot_1.bmp");
            });

            // Take second screenshot after 3 seconds
            timerSystem->SetTimeout(3.0, [this]() {
                TakeScreenshot("screenshot_2.bmp");
            });

            // Take third screenshot after 5 seconds and quit
            timerSystem->SetTimeout(5.0, [this]() {
                TakeScreenshot("screenshot_3.bmp");
                TOYFRAMEV_LOG_INFO("Demo complete! Screenshots saved.");
                Quit();
            });

            TOYFRAMEV_LOG_INFO("Scheduled 3 screenshots at 1s, 3s, 5s");
        }

        return true;
    }

    void OnUpdate(float deltaTime) override {
        m_time += deltaTime;
    }

    void OnRender() override {
        auto* graphics = GetGraphics();

        // First, render to the RenderTexture
        graphics->SetRenderTarget(m_renderTexture.get());
        {
            // Animate color based on time
            float r = (std::sin(m_time * 2.0f) + 1.0f) * 0.5f;
            float g = (std::sin(m_time * 3.0f) + 1.0f) * 0.5f;
            float b = (std::sin(m_time * 1.5f) + 1.0f) * 0.5f;
            
            graphics->Clear(Color(r, g, b));
            
            // TODO: Draw actual geometry here when we have more graphics features
            // For now, we just render animated colors
        }

        // Switch back to screen rendering
        graphics->SetRenderTarget(nullptr);
        {
            // Clear screen with a different color to show we're rendering to screen
            graphics->Clear(Color::CornflowerBlue());
            
            // In a full implementation, we would render the RenderTexture
            // as a textured quad here. For now, just show screen color.
        }
    }

    void OnShutdown() override {
        m_renderTexture.reset();
        std::cout << "HelloRenderTexture shutdown!" << std::endl;
    }

private:
    void TakeScreenshot(const std::string& filename) {
        if (!m_renderTexture) return;

        TOYFRAMEV_LOG_INFO("Taking screenshot: {}", filename);

        // Read pixels from RenderTexture (synchronous)
        PixelData pixels = m_renderTexture->ReadPixels();
        
        if (pixels.IsValid()) {
            if (pixels.SaveToBMP(filename)) {
                TOYFRAMEV_LOG_INFO("Saved screenshot: {} ({}x{})", 
                    filename, pixels.width, pixels.height);
            } else {
                TOYFRAMEV_LOG_ERROR("Failed to save screenshot: {}", filename);
            }
        } else {
            TOYFRAMEV_LOG_ERROR("Failed to read pixels from RenderTexture");
        }
    }

    std::unique_ptr<RenderTexture> m_renderTexture;
    float m_time = 0.0f;
};

// Entry point
TOYFRAMEV_MAIN(HelloRenderTextureApp)
