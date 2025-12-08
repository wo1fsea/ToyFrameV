/**
 * @file InputLinux.cpp
 * @brief Linux X11 KeySym to KeyCode mapping
 */

#include "ToyFrameV/Input.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_LINUX

#include <X11/Xlib.h>
#include <X11/keysym.h>

namespace ToyFrameV {

/**
 * @brief Convert X11 KeySym to ToyFrameV KeyCode
 */
KeyCode X11KeyToKeyCode(KeySym keysym) {
    switch (keysym) {
        // Letters
        case XK_a: case XK_A: return KeyCode::A;
        case XK_b: case XK_B: return KeyCode::B;
        case XK_c: case XK_C: return KeyCode::C;
        case XK_d: case XK_D: return KeyCode::D;
        case XK_e: case XK_E: return KeyCode::E;
        case XK_f: case XK_F: return KeyCode::F;
        case XK_g: case XK_G: return KeyCode::G;
        case XK_h: case XK_H: return KeyCode::H;
        case XK_i: case XK_I: return KeyCode::I;
        case XK_j: case XK_J: return KeyCode::J;
        case XK_k: case XK_K: return KeyCode::K;
        case XK_l: case XK_L: return KeyCode::L;
        case XK_m: case XK_M: return KeyCode::M;
        case XK_n: case XK_N: return KeyCode::N;
        case XK_o: case XK_O: return KeyCode::O;
        case XK_p: case XK_P: return KeyCode::P;
        case XK_q: case XK_Q: return KeyCode::Q;
        case XK_r: case XK_R: return KeyCode::R;
        case XK_s: case XK_S: return KeyCode::S;
        case XK_t: case XK_T: return KeyCode::T;
        case XK_u: case XK_U: return KeyCode::U;
        case XK_v: case XK_V: return KeyCode::V;
        case XK_w: case XK_W: return KeyCode::W;
        case XK_x: case XK_X: return KeyCode::X;
        case XK_y: case XK_Y: return KeyCode::Y;
        case XK_z: case XK_Z: return KeyCode::Z;

        // Numbers
        case XK_0: return KeyCode::Num0;
        case XK_1: return KeyCode::Num1;
        case XK_2: return KeyCode::Num2;
        case XK_3: return KeyCode::Num3;
        case XK_4: return KeyCode::Num4;
        case XK_5: return KeyCode::Num5;
        case XK_6: return KeyCode::Num6;
        case XK_7: return KeyCode::Num7;
        case XK_8: return KeyCode::Num8;
        case XK_9: return KeyCode::Num9;

        // Function keys
        case XK_F1: return KeyCode::F1;
        case XK_F2: return KeyCode::F2;
        case XK_F3: return KeyCode::F3;
        case XK_F4: return KeyCode::F4;
        case XK_F5: return KeyCode::F5;
        case XK_F6: return KeyCode::F6;
        case XK_F7: return KeyCode::F7;
        case XK_F8: return KeyCode::F8;
        case XK_F9: return KeyCode::F9;
        case XK_F10: return KeyCode::F10;
        case XK_F11: return KeyCode::F11;
        case XK_F12: return KeyCode::F12;

        // Arrow keys
        case XK_Left: return KeyCode::Left;
        case XK_Right: return KeyCode::Right;
        case XK_Up: return KeyCode::Up;
        case XK_Down: return KeyCode::Down;

        // Special keys
        case XK_Escape: return KeyCode::Escape;
        case XK_Return: return KeyCode::Return;
        case XK_Tab: return KeyCode::Tab;
        case XK_BackSpace: return KeyCode::Backspace;
        case XK_Insert: return KeyCode::Insert;
        case XK_Delete: return KeyCode::Delete;
        case XK_Home: return KeyCode::Home;
        case XK_End: return KeyCode::End;
        case XK_Page_Up: return KeyCode::PageUp;
        case XK_Page_Down: return KeyCode::PageDown;

        // Modifiers
        case XK_Shift_L: return KeyCode::LeftShift;
        case XK_Shift_R: return KeyCode::RightShift;
        case XK_Control_L: return KeyCode::LeftCtrl;
        case XK_Control_R: return KeyCode::RightCtrl;
        case XK_Alt_L: return KeyCode::LeftAlt;
        case XK_Alt_R: return KeyCode::RightAlt;
        case XK_Super_L: return KeyCode::LeftSuper;
        case XK_Super_R: return KeyCode::RightSuper;
        case XK_space: return KeyCode::Space;

        // Punctuation
        case XK_apostrophe: return KeyCode::Apostrophe;
        case XK_comma: return KeyCode::Comma;
        case XK_minus: return KeyCode::Minus;
        case XK_period: return KeyCode::Period;
        case XK_slash: return KeyCode::Slash;
        case XK_semicolon: return KeyCode::Semicolon;
        case XK_equal: return KeyCode::Equals;
        case XK_bracketleft: return KeyCode::LeftBracket;
        case XK_backslash: return KeyCode::Backslash;
        case XK_bracketright: return KeyCode::RightBracket;
        case XK_grave: return KeyCode::Grave;

        // Numpad
        case XK_KP_0: return KeyCode::Numpad0;
        case XK_KP_1: return KeyCode::Numpad1;
        case XK_KP_2: return KeyCode::Numpad2;
        case XK_KP_3: return KeyCode::Numpad3;
        case XK_KP_4: return KeyCode::Numpad4;
        case XK_KP_5: return KeyCode::Numpad5;
        case XK_KP_6: return KeyCode::Numpad6;
        case XK_KP_7: return KeyCode::Numpad7;
        case XK_KP_8: return KeyCode::Numpad8;
        case XK_KP_9: return KeyCode::Numpad9;
        case XK_KP_Decimal: return KeyCode::NumpadDecimal;
        case XK_KP_Divide: return KeyCode::NumpadDivide;
        case XK_KP_Multiply: return KeyCode::NumpadMultiply;
        case XK_KP_Subtract: return KeyCode::NumpadMinus;
        case XK_KP_Add: return KeyCode::NumpadPlus;
        case XK_KP_Enter: return KeyCode::NumpadEnter;

        default:
            return KeyCode::Unknown;
    }
}

} // namespace ToyFrameV

#endif // PLATFORM_LINUX
