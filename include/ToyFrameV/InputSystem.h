#pragma once

/**
 * @file InputSystem.h
 * @brief Input subsystem for ToyFrameV framework
 * 
 * Manages input state updates per frame.
 * Depends on WindowSystem for receiving input events.
 */

#include "ToyFrameV/System.h"
#include "ToyFrameV/Input.h"

namespace ToyFrameV {

// Forward declarations
class App;
class WindowSystem;

/**
 * @brief Input subsystem
 * 
 * Handles per-frame input state management:
 * - PreUpdate: saves previous frame state
 * - PostUpdate: resets delta values (scroll)
 * 
 * The actual input state is updated by WindowSystem's event callbacks
 * calling Input::_SetXxxState functions.
 */
class InputSystem : public System {
public:
    InputSystem() = default;
    ~InputSystem() override = default;

    // System interface
    const char* GetName() const override { return "InputSystem"; }
    int GetPriority() const override { return static_cast<int>(SystemPriority::Input); }
    
    std::vector<const std::type_info*> GetDependencies() const override;

    bool Initialize(App* app) override;
    void PreUpdate() override;
    void PostUpdate() override;
    void Shutdown() override;

    // Convenience static wrappers (delegate to Input class)
    /**
     * @brief Check if a key is currently pressed
     */
    static bool IsKeyDown(KeyCode key) { return Input::IsKeyDown(key); }

    /**
     * @brief Check if a key was just pressed this frame
     */
    static bool IsKeyPressed(KeyCode key) { return Input::IsKeyPressed(key); }

    /**
     * @brief Check if a key was just released this frame
     */
    static bool IsKeyReleased(KeyCode key) { return Input::IsKeyReleased(key); }

    /**
     * @brief Check if a mouse button is currently pressed
     */
    static bool IsMouseButtonDown(MouseButton button) { return Input::IsMouseButtonDown(button); }

    /**
     * @brief Check if a mouse button was just pressed this frame
     */
    static bool IsMouseButtonPressed(MouseButton button) { return Input::IsMouseButtonPressed(button); }

    /**
     * @brief Check if a mouse button was just released this frame
     */
    static bool IsMouseButtonReleased(MouseButton button) { return Input::IsMouseButtonReleased(button); }

    /**
     * @brief Get current mouse position
     */
    static void GetMousePosition(int& x, int& y) { Input::GetMousePosition(x, y); }

    /**
     * @brief Get mouse X position
     */
    static int GetMouseX() { return Input::GetMouseX(); }

    /**
     * @brief Get mouse Y position
     */
    static int GetMouseY() { return Input::GetMouseY(); }

    /**
     * @brief Get scroll delta
     */
    static void GetScrollDelta(float& x, float& y) { Input::GetScrollDelta(x, y); }

    /**
     * @brief Check if shift is held
     */
    static bool IsShiftDown() { return Input::IsShiftDown(); }

    /**
     * @brief Check if ctrl is held
     */
    static bool IsCtrlDown() { return Input::IsCtrlDown(); }

    /**
     * @brief Check if alt is held
     */
    static bool IsAltDown() { return Input::IsAltDown(); }
};

} // namespace ToyFrameV
