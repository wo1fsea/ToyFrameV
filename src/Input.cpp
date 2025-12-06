#include "ToyFrameV/Input.h"

namespace ToyFrameV {

// Static member definitions
std::bitset<Input::KEY_COUNT> Input::s_currentKeys;
std::bitset<Input::KEY_COUNT> Input::s_previousKeys;
std::bitset<Input::MOUSE_BUTTON_COUNT> Input::s_currentMouseButtons;
std::bitset<Input::MOUSE_BUTTON_COUNT> Input::s_previousMouseButtons;

int Input::s_mouseX = 0;
int Input::s_mouseY = 0;
float Input::s_scrollX = 0.0f;
float Input::s_scrollY = 0.0f;
bool Input::s_shift = false;
bool Input::s_ctrl = false;
bool Input::s_alt = false;

bool Input::IsKeyDown(KeyCode key) {
    size_t index = static_cast<size_t>(key);
    if (index >= KEY_COUNT) return false;
    return s_currentKeys[index];
}

bool Input::IsKeyPressed(KeyCode key) {
    size_t index = static_cast<size_t>(key);
    if (index >= KEY_COUNT) return false;
    return s_currentKeys[index] && !s_previousKeys[index];
}

bool Input::IsKeyReleased(KeyCode key) {
    size_t index = static_cast<size_t>(key);
    if (index >= KEY_COUNT) return false;
    return !s_currentKeys[index] && s_previousKeys[index];
}

bool Input::IsMouseButtonDown(MouseButton button) {
    size_t index = static_cast<size_t>(button);
    if (index >= MOUSE_BUTTON_COUNT) return false;
    return s_currentMouseButtons[index];
}

bool Input::IsMouseButtonPressed(MouseButton button) {
    size_t index = static_cast<size_t>(button);
    if (index >= MOUSE_BUTTON_COUNT) return false;
    return s_currentMouseButtons[index] && !s_previousMouseButtons[index];
}

bool Input::IsMouseButtonReleased(MouseButton button) {
    size_t index = static_cast<size_t>(button);
    if (index >= MOUSE_BUTTON_COUNT) return false;
    return !s_currentMouseButtons[index] && s_previousMouseButtons[index];
}

void Input::GetMousePosition(int& x, int& y) {
    x = s_mouseX;
    y = s_mouseY;
}

int Input::GetMouseX() {
    return s_mouseX;
}

int Input::GetMouseY() {
    return s_mouseY;
}

void Input::GetScrollDelta(float& x, float& y) {
    x = s_scrollX;
    y = s_scrollY;
}

bool Input::IsShiftDown() {
    return s_shift;
}

bool Input::IsCtrlDown() {
    return s_ctrl;
}

bool Input::IsAltDown() {
    return s_alt;
}

void Input::_SetKeyState(KeyCode key, bool pressed) {
    size_t index = static_cast<size_t>(key);
    if (index < KEY_COUNT) {
        s_currentKeys[index] = pressed;
    }
}

void Input::_SetMouseButtonState(MouseButton button, bool pressed) {
    size_t index = static_cast<size_t>(button);
    if (index < MOUSE_BUTTON_COUNT) {
        s_currentMouseButtons[index] = pressed;
    }
}

void Input::_SetMousePosition(int x, int y) {
    s_mouseX = x;
    s_mouseY = y;
}

void Input::_SetScrollDelta(float x, float y) {
    s_scrollX = x;
    s_scrollY = y;
}

void Input::_SetModifiers(bool shift, bool ctrl, bool alt) {
    s_shift = shift;
    s_ctrl = ctrl;
    s_alt = alt;
}

void Input::_UpdatePreviousState() {
    s_previousKeys = s_currentKeys;
    s_previousMouseButtons = s_currentMouseButtons;
}

void Input::_ResetScrollDelta() {
    s_scrollX = 0.0f;
    s_scrollY = 0.0f;
}

} // namespace ToyFrameV
