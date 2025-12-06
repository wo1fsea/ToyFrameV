#pragma once

#include <cstdint>

namespace ToyFrameV {

/**
 * @brief Platform-independent key codes
 * 
 * Based on USB HID usage tables with extensions for common keys.
 */
enum class KeyCode : uint16_t {
    Unknown = 0,

    // Letters
    A = 4, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Numbers (top row)
    Num1 = 30, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9, Num0,

    // Function keys
    Return = 40,
    Escape = 41,
    Backspace = 42,
    Tab = 43,
    Space = 44,

    // Symbols
    Minus = 45,          // -
    Equals = 46,         // =
    LeftBracket = 47,    // [
    RightBracket = 48,   // ]
    Backslash = 49,      // '\'
    Semicolon = 51,      // ;
    Apostrophe = 52,     // '
    Grave = 53,          // `
    Comma = 54,          // ,
    Period = 55,         // .
    Slash = 56,          // /

    // Lock keys
    CapsLock = 57,
    
    // Function keys
    F1 = 58, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,

    // Navigation
    PrintScreen = 70,
    ScrollLock = 71,
    Pause = 72,
    Insert = 73,
    Home = 74,
    PageUp = 75,
    Delete = 76,
    End = 77,
    PageDown = 78,

    // Arrow keys
    Right = 79,
    Left = 80,
    Down = 81,
    Up = 82,

    // Numpad
    NumLock = 83,
    NumpadDivide = 84,
    NumpadMultiply = 85,
    NumpadMinus = 86,
    NumpadPlus = 87,
    NumpadEnter = 88,
    Numpad1 = 89, Numpad2, Numpad3, Numpad4, Numpad5,
    Numpad6, Numpad7, Numpad8, Numpad9, Numpad0,
    NumpadDecimal = 99,

    // Modifiers
    LeftCtrl = 224,
    LeftShift = 225,
    LeftAlt = 226,
    LeftSuper = 227,    // Windows key / Command
    RightCtrl = 228,
    RightShift = 229,
    RightAlt = 230,
    RightSuper = 231,

    // Additional keys
    Menu = 101,         // Application/Context menu key

    Count = 256
};

/**
 * @brief Get human-readable name for a key code
 */
inline const char* GetKeyName(KeyCode key) {
    switch (key) {
        case KeyCode::A: return "A";
        case KeyCode::B: return "B";
        case KeyCode::C: return "C";
        case KeyCode::D: return "D";
        case KeyCode::E: return "E";
        case KeyCode::F: return "F";
        case KeyCode::G: return "G";
        case KeyCode::H: return "H";
        case KeyCode::I: return "I";
        case KeyCode::J: return "J";
        case KeyCode::K: return "K";
        case KeyCode::L: return "L";
        case KeyCode::M: return "M";
        case KeyCode::N: return "N";
        case KeyCode::O: return "O";
        case KeyCode::P: return "P";
        case KeyCode::Q: return "Q";
        case KeyCode::R: return "R";
        case KeyCode::S: return "S";
        case KeyCode::T: return "T";
        case KeyCode::U: return "U";
        case KeyCode::V: return "V";
        case KeyCode::W: return "W";
        case KeyCode::X: return "X";
        case KeyCode::Y: return "Y";
        case KeyCode::Z: return "Z";
        case KeyCode::Num0: return "0";
        case KeyCode::Num1: return "1";
        case KeyCode::Num2: return "2";
        case KeyCode::Num3: return "3";
        case KeyCode::Num4: return "4";
        case KeyCode::Num5: return "5";
        case KeyCode::Num6: return "6";
        case KeyCode::Num7: return "7";
        case KeyCode::Num8: return "8";
        case KeyCode::Num9: return "9";
        case KeyCode::Space: return "Space";
        case KeyCode::Return: return "Return";
        case KeyCode::Escape: return "Escape";
        case KeyCode::Tab: return "Tab";
        case KeyCode::Backspace: return "Backspace";
        case KeyCode::Delete: return "Delete";
        case KeyCode::Insert: return "Insert";
        case KeyCode::Home: return "Home";
        case KeyCode::End: return "End";
        case KeyCode::PageUp: return "PageUp";
        case KeyCode::PageDown: return "PageDown";
        case KeyCode::Up: return "Up";
        case KeyCode::Down: return "Down";
        case KeyCode::Left: return "Left";
        case KeyCode::Right: return "Right";
        case KeyCode::F1: return "F1";
        case KeyCode::F2: return "F2";
        case KeyCode::F3: return "F3";
        case KeyCode::F4: return "F4";
        case KeyCode::F5: return "F5";
        case KeyCode::F6: return "F6";
        case KeyCode::F7: return "F7";
        case KeyCode::F8: return "F8";
        case KeyCode::F9: return "F9";
        case KeyCode::F10: return "F10";
        case KeyCode::F11: return "F11";
        case KeyCode::F12: return "F12";
        case KeyCode::LeftShift: return "LShift";
        case KeyCode::RightShift: return "RShift";
        case KeyCode::LeftCtrl: return "LCtrl";
        case KeyCode::RightCtrl: return "RCtrl";
        case KeyCode::LeftAlt: return "LAlt";
        case KeyCode::RightAlt: return "RAlt";
        case KeyCode::LeftSuper: return "LSuper";
        case KeyCode::RightSuper: return "RSuper";
        default: return "Unknown";
    }
}

} // namespace ToyFrameV
