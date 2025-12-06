/**
 * @file InputWindows.cpp
 * @brief Windows virtual key to KeyCode mapping
 */

#include "ToyFrameV/Input.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace ToyFrameV {

/**
 * @brief Convert Windows virtual key code to ToyFrameV KeyCode
 */
KeyCode VirtualKeyToKeyCode(WPARAM vk, LPARAM lParam) {
    // Extended key flag
    bool extended = (lParam & 0x01000000) != 0;

    switch (vk) {
        // Letters
        case 'A': return KeyCode::A;
        case 'B': return KeyCode::B;
        case 'C': return KeyCode::C;
        case 'D': return KeyCode::D;
        case 'E': return KeyCode::E;
        case 'F': return KeyCode::F;
        case 'G': return KeyCode::G;
        case 'H': return KeyCode::H;
        case 'I': return KeyCode::I;
        case 'J': return KeyCode::J;
        case 'K': return KeyCode::K;
        case 'L': return KeyCode::L;
        case 'M': return KeyCode::M;
        case 'N': return KeyCode::N;
        case 'O': return KeyCode::O;
        case 'P': return KeyCode::P;
        case 'Q': return KeyCode::Q;
        case 'R': return KeyCode::R;
        case 'S': return KeyCode::S;
        case 'T': return KeyCode::T;
        case 'U': return KeyCode::U;
        case 'V': return KeyCode::V;
        case 'W': return KeyCode::W;
        case 'X': return KeyCode::X;
        case 'Y': return KeyCode::Y;
        case 'Z': return KeyCode::Z;

        // Numbers
        case '0': return KeyCode::Num0;
        case '1': return KeyCode::Num1;
        case '2': return KeyCode::Num2;
        case '3': return KeyCode::Num3;
        case '4': return KeyCode::Num4;
        case '5': return KeyCode::Num5;
        case '6': return KeyCode::Num6;
        case '7': return KeyCode::Num7;
        case '8': return KeyCode::Num8;
        case '9': return KeyCode::Num9;

        // Function keys
        case VK_F1: return KeyCode::F1;
        case VK_F2: return KeyCode::F2;
        case VK_F3: return KeyCode::F3;
        case VK_F4: return KeyCode::F4;
        case VK_F5: return KeyCode::F5;
        case VK_F6: return KeyCode::F6;
        case VK_F7: return KeyCode::F7;
        case VK_F8: return KeyCode::F8;
        case VK_F9: return KeyCode::F9;
        case VK_F10: return KeyCode::F10;
        case VK_F11: return KeyCode::F11;
        case VK_F12: return KeyCode::F12;

        // Special keys
        case VK_ESCAPE: return KeyCode::Escape;
        case VK_TAB: return KeyCode::Tab;
        case VK_CAPITAL: return KeyCode::CapsLock;
        case VK_SPACE: return KeyCode::Space;
        case VK_BACK: return KeyCode::Backspace;
        case VK_RETURN: return extended ? KeyCode::NumpadEnter : KeyCode::Return;

        // Navigation
        case VK_INSERT: return KeyCode::Insert;
        case VK_DELETE: return KeyCode::Delete;
        case VK_HOME: return KeyCode::Home;
        case VK_END: return KeyCode::End;
        case VK_PRIOR: return KeyCode::PageUp;
        case VK_NEXT: return KeyCode::PageDown;

        // Arrow keys
        case VK_UP: return KeyCode::Up;
        case VK_DOWN: return KeyCode::Down;
        case VK_LEFT: return KeyCode::Left;
        case VK_RIGHT: return KeyCode::Right;

        // Modifiers
        case VK_SHIFT:
            return (MapVirtualKeyW((lParam >> 16) & 0xFF, MAPVK_VSC_TO_VK_EX) == VK_RSHIFT) 
                   ? KeyCode::RightShift : KeyCode::LeftShift;
        case VK_CONTROL:
            return extended ? KeyCode::RightCtrl : KeyCode::LeftCtrl;
        case VK_MENU:
            return extended ? KeyCode::RightAlt : KeyCode::LeftAlt;
        case VK_LWIN: return KeyCode::LeftSuper;
        case VK_RWIN: return KeyCode::RightSuper;

        // Symbols
        case VK_OEM_MINUS: return KeyCode::Minus;
        case VK_OEM_PLUS: return KeyCode::Equals;
        case VK_OEM_4: return KeyCode::LeftBracket;
        case VK_OEM_6: return KeyCode::RightBracket;
        case VK_OEM_5: return KeyCode::Backslash;
        case VK_OEM_1: return KeyCode::Semicolon;
        case VK_OEM_7: return KeyCode::Apostrophe;
        case VK_OEM_3: return KeyCode::Grave;
        case VK_OEM_COMMA: return KeyCode::Comma;
        case VK_OEM_PERIOD: return KeyCode::Period;
        case VK_OEM_2: return KeyCode::Slash;

        // Numpad
        case VK_NUMLOCK: return KeyCode::NumLock;
        case VK_DIVIDE: return KeyCode::NumpadDivide;
        case VK_MULTIPLY: return KeyCode::NumpadMultiply;
        case VK_SUBTRACT: return KeyCode::NumpadMinus;
        case VK_ADD: return KeyCode::NumpadPlus;
        case VK_NUMPAD0: return KeyCode::Numpad0;
        case VK_NUMPAD1: return KeyCode::Numpad1;
        case VK_NUMPAD2: return KeyCode::Numpad2;
        case VK_NUMPAD3: return KeyCode::Numpad3;
        case VK_NUMPAD4: return KeyCode::Numpad4;
        case VK_NUMPAD5: return KeyCode::Numpad5;
        case VK_NUMPAD6: return KeyCode::Numpad6;
        case VK_NUMPAD7: return KeyCode::Numpad7;
        case VK_NUMPAD8: return KeyCode::Numpad8;
        case VK_NUMPAD9: return KeyCode::Numpad9;
        case VK_DECIMAL: return KeyCode::NumpadDecimal;

        // Other
        case VK_SNAPSHOT: return KeyCode::PrintScreen;
        case VK_SCROLL: return KeyCode::ScrollLock;
        case VK_PAUSE: return KeyCode::Pause;
        case VK_APPS: return KeyCode::Menu;

        default: return KeyCode::Unknown;
    }
}

} // namespace ToyFrameV

#endif // PLATFORM_WINDOWS
