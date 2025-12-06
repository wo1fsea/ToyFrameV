#pragma once

#include "ToyFrameV/KeyCodes.h"
#include <functional>
#include <array>
#include <bitset>

namespace ToyFrameV {

/**
 * @brief Mouse button enumeration
 */
enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Count = 5
};

/**
 * @brief Input event types
 */
enum class InputEventType {
    KeyDown,
    KeyUp,
    KeyRepeat,
    MouseButtonDown,
    MouseButtonUp,
    MouseMove,
    MouseScroll,
    // Touch events for mobile
    TouchBegin,
    TouchMove,
    TouchEnd
};

/**
 * @brief Keyboard event data
 */
struct KeyEvent {
    KeyCode key;
    int scancode;
    bool shift;
    bool ctrl;
    bool alt;
    bool repeat;
};

/**
 * @brief Mouse button event data
 */
struct MouseButtonEvent {
    MouseButton button;
    int x;
    int y;
    bool shift;
    bool ctrl;
    bool alt;
};

/**
 * @brief Mouse move event data
 */
struct MouseMoveEvent {
    int x;
    int y;
    int deltaX;
    int deltaY;
};

/**
 * @brief Mouse scroll event data
 */
struct MouseScrollEvent {
    float deltaX;
    float deltaY;
    int x;
    int y;
};

/**
 * @brief Touch event data
 */
struct TouchEvent {
    int id;         // Touch point ID
    float x;
    float y;
    float pressure;
};

/**
 * @brief Input event union
 */
struct InputEvent {
    InputEventType type;
    union {
        KeyEvent key;
        MouseButtonEvent mouseButton;
        MouseMoveEvent mouseMove;
        MouseScrollEvent mouseScroll;
        TouchEvent touch;
    };

    InputEvent() : type(InputEventType::KeyDown) {
        key = {};
    }
};

/**
 * @brief Input event callback
 */
using InputEventCallback = std::function<void(const InputEvent&)>;

/**
 * @brief Input state manager
 * 
 * Tracks current state of keyboard and mouse for polling-based input.
 */
class Input {
public:
    /**
     * @brief Check if a key is currently pressed
     */
    static bool IsKeyDown(KeyCode key);

    /**
     * @brief Check if a key was just pressed this frame
     */
    static bool IsKeyPressed(KeyCode key);

    /**
     * @brief Check if a key was just released this frame
     */
    static bool IsKeyReleased(KeyCode key);

    /**
     * @brief Check if a mouse button is currently pressed
     */
    static bool IsMouseButtonDown(MouseButton button);

    /**
     * @brief Check if a mouse button was just pressed this frame
     */
    static bool IsMouseButtonPressed(MouseButton button);

    /**
     * @brief Check if a mouse button was just released this frame
     */
    static bool IsMouseButtonReleased(MouseButton button);

    /**
     * @brief Get current mouse position
     */
    static void GetMousePosition(int& x, int& y);

    /**
     * @brief Get mouse position X
     */
    static int GetMouseX();

    /**
     * @brief Get mouse position Y
     */
    static int GetMouseY();

    /**
     * @brief Get mouse scroll delta
     */
    static void GetScrollDelta(float& x, float& y);

    /**
     * @brief Check if shift is held
     */
    static bool IsShiftDown();

    /**
     * @brief Check if ctrl is held
     */
    static bool IsCtrlDown();

    /**
     * @brief Check if alt is held
     */
    static bool IsAltDown();

    // Internal functions - called by Window implementation
    static void _SetKeyState(KeyCode key, bool pressed);
    static void _SetMouseButtonState(MouseButton button, bool pressed);
    static void _SetMousePosition(int x, int y);
    static void _SetScrollDelta(float x, float y);
    static void _SetModifiers(bool shift, bool ctrl, bool alt);
    static void _UpdatePreviousState();
    static void _ResetScrollDelta();

private:
    static constexpr size_t KEY_COUNT = static_cast<size_t>(KeyCode::Count);
    static constexpr size_t MOUSE_BUTTON_COUNT = static_cast<size_t>(MouseButton::Count);

    static std::bitset<KEY_COUNT> s_currentKeys;
    static std::bitset<KEY_COUNT> s_previousKeys;
    static std::bitset<MOUSE_BUTTON_COUNT> s_currentMouseButtons;
    static std::bitset<MOUSE_BUTTON_COUNT> s_previousMouseButtons;
    
    static int s_mouseX;
    static int s_mouseY;
    static float s_scrollX;
    static float s_scrollY;
    static bool s_shift;
    static bool s_ctrl;
    static bool s_alt;
};

} // namespace ToyFrameV
