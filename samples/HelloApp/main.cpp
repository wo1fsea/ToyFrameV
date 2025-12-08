/**
 * @file main.cpp
 * @brief HelloApp - Basic ToyFrameV sample
 * 
 * This sample shows a basic ToyFrameV application with graphics rendering.
 */

#include <ToyFrameV.h>
#include <iostream>
#include <cmath>

using namespace ToyFrameV;

class HelloApp : public App {
public:
    HelloApp() {
        m_config.title = "Hello ToyFrameV";
        m_config.windowWidth = 800;
        m_config.windowHeight = 600;
    }

protected:
    bool OnInit() override {
        std::cout << "========================================" << std::endl;
        std::cout << "    HelloApp - Basic Demo" << std::endl;
        std::cout << "    ToyFrameV Version: " 
                  << TOYFRAMEV_VERSION_MAJOR << "."
                  << TOYFRAMEV_VERSION_MINOR << "."
                  << TOYFRAMEV_VERSION_PATCH << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Backend: " << GetGraphics()->GetBackendName() << std::endl;
        std::cout << "Device: " << GetGraphics()->GetDeviceName() << std::endl;
        std::cout << "========================================" << std::endl;
        return true;
    }

    void OnUpdate(float deltaTime) override {
        m_time += deltaTime;
    }

    void OnRender() override {
        // Animate background color
        float r = (std::sin(m_time * 0.5f) + 1.0f) * 0.2f;
        float g = (std::sin(m_time * 0.7f) + 1.0f) * 0.2f;
        float b = (std::sin(m_time * 0.3f) + 1.0f) * 0.3f;
        
        GetGraphics()->Clear(Color(r, g, b));
    }

    void OnShutdown() override {
        std::cout << "HelloApp shutdown!" << std::endl;
    }

private:
    float m_time = 0.0f;
};

// Entry point
TOYFRAMEV_MAIN(HelloApp)
