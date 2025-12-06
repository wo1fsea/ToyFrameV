#pragma once

/**
 * @file GraphicsSystem.h
 * @brief Graphics subsystem for ToyFrameV framework
 * 
 * Manages rendering context, swap chain, and provides rendering API.
 * Depends on WindowSystem for the native window handle.
 */

#include "ToyFrameV/System.h"
#include "ToyFrameV/Graphics.h"
#include <memory>

namespace ToyFrameV {

// Forward declarations
class App;
class WindowSystem;

/**
 * @brief Graphics subsystem
 * 
 * Handles rendering context creation and frame management.
 * Depends on WindowSystem for the native window handle.
 * 
 * Note: Currently LLGL creates its own window internally, but
 * this system registers for resize events from WindowSystem
 * for future compatibility.
 */
class GraphicsSystem : public System {
public:
    GraphicsSystem() = default;
    explicit GraphicsSystem(const GraphicsConfig& config);
    ~GraphicsSystem() override = default;

    // System interface
    const char* GetName() const override { return "GraphicsSystem"; }
    int GetPriority() const override { return static_cast<int>(SystemPriority::Present); }
    
    std::vector<const std::type_info*> GetDependencies() const override;

    bool Initialize(App* app) override;
    void PreUpdate() override;
    void Render() override;
    void PostUpdate() override;
    void Shutdown() override;

    // Graphics access
    /**
     * @brief Get the underlying Graphics instance
     */
    Graphics* GetGraphics() const { return m_graphics.get(); }

    /**
     * @brief Get graphics configuration
     */
    const GraphicsConfig& GetConfig() const { return m_config; }

    /**
     * @brief Check if graphics context is valid
     */
    bool IsValid() const { return m_graphics != nullptr; }

    // Convenience wrappers
    /**
     * @brief Clear the render target
     */
    void Clear(const Color& color);

    /**
     * @brief Get backend name
     */
    const std::string& GetBackendName() const;

    /**
     * @brief Get device name
     */
    const std::string& GetDeviceName() const;

private:
    void OnResize(int width, int height);

    GraphicsConfig m_config;
    std::unique_ptr<Graphics> m_graphics;
    bool m_frameStarted = false;
};

} // namespace ToyFrameV
