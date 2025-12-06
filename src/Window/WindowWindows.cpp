/**
 * @file WindowWindows.cpp
 * @brief Windows platform window implementation using Win32 API
 */

#include "ToyFrameV/Window.h"
#include "ToyFrameV/Input.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM
#include <unordered_map>
#include <iostream>

namespace ToyFrameV {

// Forward declaration of key mapping function
KeyCode VirtualKeyToKeyCode(WPARAM vk, LPARAM lParam);

// Window class name
static const wchar_t* WINDOW_CLASS_NAME = L"ToyFrameVWindowClass";
static bool s_windowClassRegistered = false;

// Map HWND to Window instance for message routing
static std::unordered_map<HWND, class WindowWindows*> s_windowMap;

/**
 * @brief Windows-specific window implementation
 */
class WindowWindows : public Window {
public:
    WindowWindows(const WindowConfig& config);
    ~WindowWindows() override;

    bool ProcessEvents() override;
    void Close() override;
    bool IsOpen() const override { return m_hwnd != nullptr && m_isOpen; }
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    void SetTitle(const std::string& title) override;
    void SetSize(int width, int height) override;
    void SetPosition(int x, int y) override;
    void SetVisible(bool visible) override;
    void* GetNativeHandle() const override { return m_hwnd; }

    // Windows message handler
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool RegisterWindowClass();
    bool CreateNativeWindow(const WindowConfig& config);
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd = nullptr;
    int m_width = 0;
    int m_height = 0;
    bool m_isOpen = false;
};

// Static window procedure - routes messages to instance
LRESULT CALLBACK WindowWindows::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WindowWindows* window = nullptr;

    if (msg == WM_NCCREATE) {
        // Store window pointer from create params
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        window = static_cast<WindowWindows*>(cs->lpCreateParams);
        window->m_hwnd = hwnd;  // Set hwnd early
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        s_windowMap[hwnd] = window;
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    } else {
        window = reinterpret_cast<WindowWindows*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (window && window->m_hwnd) {
        return window->HandleMessage(msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

WindowWindows::WindowWindows(const WindowConfig& config) {
    if (!RegisterWindowClass()) {
        return;
    }

    if (!CreateNativeWindow(config)) {
        return;
    }

    m_isOpen = true;
}

WindowWindows::~WindowWindows() {
    Close();
}

bool WindowWindows::RegisterWindowClass() {
    if (s_windowClassRegistered) {
        return true;
    }

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));  // IDC_ARROW
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIcon = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));      // IDI_APPLICATION
    wc.hIconSm = LoadIconW(nullptr, MAKEINTRESOURCEW(32512));    // IDI_APPLICATION

    if (!RegisterClassExW(&wc)) {
        DWORD error = GetLastError();
        std::cerr << "RegisterClassExW failed with error: " << error << std::endl;
        return false;
    }

    s_windowClassRegistered = true;
    return true;
}

bool WindowWindows::CreateNativeWindow(const WindowConfig& config) {
    // Convert title to wide string
    int titleLen = MultiByteToWideChar(CP_UTF8, 0, config.title.c_str(), -1, nullptr, 0);
    std::wstring wideTitle(titleLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, config.title.c_str(), -1, &wideTitle[0], titleLen);

    // Window style
    DWORD style = WS_OVERLAPPEDWINDOW;
    if (!config.resizable) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }

    // Calculate window rect to account for borders
    RECT rect = { 0, 0, config.width, config.height };
    AdjustWindowRectEx(&rect, style, FALSE, 0);

    int windowWidth = rect.right - rect.left;
    int windowHeight = rect.bottom - rect.top;

    // Position
    int posX = config.posX;
    int posY = config.posY;

    if (config.centered) {
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        posX = (screenWidth - windowWidth) / 2;
        posY = (screenHeight - windowHeight) / 2;
    }

    // Create window
    m_hwnd = CreateWindowExW(
        0,
        WINDOW_CLASS_NAME,
        wideTitle.c_str(),
        style,
        posX, posY,
        windowWidth, windowHeight,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this  // Pass this pointer to WM_NCCREATE
    );

    if (!m_hwnd) {
        DWORD error = GetLastError();
        std::cerr << "CreateWindowExW failed with error: " << error << std::endl;
        return false;
    }

    m_width = config.width;
    m_height = config.height;

    // Show window
    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);

    return true;
}

bool WindowWindows::ProcessEvents() {
    MSG msg = {};
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            m_isOpen = false;
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return m_isOpen;
}

void WindowWindows::Close() {
    if (m_hwnd) {
        s_windowMap.erase(m_hwnd);
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }
    m_isOpen = false;
}

void WindowWindows::SetTitle(const std::string& title) {
    if (!m_hwnd) return;
    
    int titleLen = MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, nullptr, 0);
    std::wstring wideTitle(titleLen, 0);
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), -1, &wideTitle[0], titleLen);
    SetWindowTextW(m_hwnd, wideTitle.c_str());
}

void WindowWindows::SetSize(int width, int height) {
    if (!m_hwnd) return;

    DWORD style = GetWindowLongW(m_hwnd, GWL_STYLE);
    RECT rect = { 0, 0, width, height };
    AdjustWindowRectEx(&rect, style, FALSE, 0);

    SetWindowPos(m_hwnd, nullptr, 0, 0, 
                 rect.right - rect.left, rect.bottom - rect.top,
                 SWP_NOMOVE | SWP_NOZORDER);

    m_width = width;
    m_height = height;
}

void WindowWindows::SetPosition(int x, int y) {
    if (!m_hwnd) return;
    SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

void WindowWindows::SetVisible(bool visible) {
    if (!m_hwnd) return;
    ShowWindow(m_hwnd, visible ? SW_SHOW : SW_HIDE);
}

LRESULT WindowWindows::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    // Update modifier state
    bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    Input::_SetModifiers(shift, ctrl, alt);

    switch (msg) {
        case WM_CLOSE: {
            WindowEvent event;
            event.type = WindowEventType::Close;
            DispatchEvent(event);
            m_isOpen = false;
            return 0;
        }

        case WM_SIZE: {
            m_width = LOWORD(lParam);
            m_height = HIWORD(lParam);

            WindowEvent event;
            event.width = m_width;
            event.height = m_height;

            if (wParam == SIZE_MINIMIZED) {
                event.type = WindowEventType::Minimize;
            } else if (wParam == SIZE_MAXIMIZED) {
                event.type = WindowEventType::Maximize;
            } else if (wParam == SIZE_RESTORED) {
                event.type = WindowEventType::Resize;
            } else {
                event.type = WindowEventType::Resize;
            }
            
            DispatchEvent(event);
            return 0;
        }

        case WM_SETFOCUS: {
            WindowEvent event;
            event.type = WindowEventType::Focus;
            DispatchEvent(event);
            return 0;
        }

        case WM_KILLFOCUS: {
            WindowEvent event;
            event.type = WindowEventType::LostFocus;
            DispatchEvent(event);
            return 0;
        }

        // Keyboard input
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            KeyCode key = VirtualKeyToKeyCode(wParam, lParam);
            bool repeat = (lParam & 0x40000000) != 0;
            
            Input::_SetKeyState(key, true);

            InputEvent event;
            event.type = repeat ? InputEventType::KeyRepeat : InputEventType::KeyDown;
            event.key.key = key;
            event.key.scancode = (lParam >> 16) & 0xFF;
            event.key.shift = shift;
            event.key.ctrl = ctrl;
            event.key.alt = alt;
            event.key.repeat = repeat;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            KeyCode key = VirtualKeyToKeyCode(wParam, lParam);
            
            Input::_SetKeyState(key, false);

            InputEvent event;
            event.type = InputEventType::KeyUp;
            event.key.key = key;
            event.key.scancode = (lParam >> 16) & 0xFF;
            event.key.shift = shift;
            event.key.ctrl = ctrl;
            event.key.alt = alt;
            event.key.repeat = false;
            DispatchInputEvent(event);
            return 0;
        }

        // Mouse button input
        case WM_LBUTTONDOWN: {
            SetCapture(m_hwnd);
            Input::_SetMouseButtonState(MouseButton::Left, true);

            InputEvent event;
            event.type = InputEventType::MouseButtonDown;
            event.mouseButton.button = MouseButton::Left;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            event.mouseButton.shift = shift;
            event.mouseButton.ctrl = ctrl;
            event.mouseButton.alt = alt;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_LBUTTONUP: {
            ReleaseCapture();
            Input::_SetMouseButtonState(MouseButton::Left, false);

            InputEvent event;
            event.type = InputEventType::MouseButtonUp;
            event.mouseButton.button = MouseButton::Left;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            event.mouseButton.shift = shift;
            event.mouseButton.ctrl = ctrl;
            event.mouseButton.alt = alt;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_RBUTTONDOWN: {
            SetCapture(m_hwnd);
            Input::_SetMouseButtonState(MouseButton::Right, true);

            InputEvent event;
            event.type = InputEventType::MouseButtonDown;
            event.mouseButton.button = MouseButton::Right;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            event.mouseButton.shift = shift;
            event.mouseButton.ctrl = ctrl;
            event.mouseButton.alt = alt;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_RBUTTONUP: {
            ReleaseCapture();
            Input::_SetMouseButtonState(MouseButton::Right, false);

            InputEvent event;
            event.type = InputEventType::MouseButtonUp;
            event.mouseButton.button = MouseButton::Right;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            event.mouseButton.shift = shift;
            event.mouseButton.ctrl = ctrl;
            event.mouseButton.alt = alt;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_MBUTTONDOWN: {
            SetCapture(m_hwnd);
            Input::_SetMouseButtonState(MouseButton::Middle, true);

            InputEvent event;
            event.type = InputEventType::MouseButtonDown;
            event.mouseButton.button = MouseButton::Middle;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            event.mouseButton.shift = shift;
            event.mouseButton.ctrl = ctrl;
            event.mouseButton.alt = alt;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_MBUTTONUP: {
            ReleaseCapture();
            Input::_SetMouseButtonState(MouseButton::Middle, false);

            InputEvent event;
            event.type = InputEventType::MouseButtonUp;
            event.mouseButton.button = MouseButton::Middle;
            event.mouseButton.x = GET_X_LPARAM(lParam);
            event.mouseButton.y = GET_Y_LPARAM(lParam);
            event.mouseButton.shift = shift;
            event.mouseButton.ctrl = ctrl;
            event.mouseButton.alt = alt;
            DispatchInputEvent(event);
            return 0;
        }

        // Mouse move
        case WM_MOUSEMOVE: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);
            
            int deltaX = x - Input::GetMouseX();
            int deltaY = y - Input::GetMouseY();
            
            Input::_SetMousePosition(x, y);

            InputEvent event;
            event.type = InputEventType::MouseMove;
            event.mouseMove.x = x;
            event.mouseMove.y = y;
            event.mouseMove.deltaX = deltaX;
            event.mouseMove.deltaY = deltaY;
            DispatchInputEvent(event);
            return 0;
        }

        // Mouse scroll
        case WM_MOUSEWHEEL: {
            float delta = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            Input::_SetScrollDelta(0.0f, delta);

            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(m_hwnd, &pt);

            InputEvent event;
            event.type = InputEventType::MouseScroll;
            event.mouseScroll.deltaX = 0.0f;
            event.mouseScroll.deltaY = delta;
            event.mouseScroll.x = pt.x;
            event.mouseScroll.y = pt.y;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_MOUSEHWHEEL: {
            float delta = GET_WHEEL_DELTA_WPARAM(wParam) / (float)WHEEL_DELTA;
            Input::_SetScrollDelta(delta, 0.0f);

            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ScreenToClient(m_hwnd, &pt);

            InputEvent event;
            event.type = InputEventType::MouseScroll;
            event.mouseScroll.deltaX = delta;
            event.mouseScroll.deltaY = 0.0f;
            event.mouseScroll.x = pt.x;
            event.mouseScroll.y = pt.y;
            DispatchInputEvent(event);
            return 0;
        }

        case WM_DESTROY: {
            if (s_windowMap.empty()) {
                PostQuitMessage(0);
            }
            return 0;
        }

        case WM_ERASEBKGND: {
            // Prevent flicker - we'll handle background in render
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(m_hwnd, &ps);
            
            // Fill with dark background for now
            HBRUSH brush = CreateSolidBrush(RGB(30, 30, 30));
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);
            
            EndPaint(m_hwnd, &ps);
            return 0;
        }
    }

    return DefWindowProcW(m_hwnd, msg, wParam, lParam);
}

// Factory function implementation
std::unique_ptr<Window> Window::Create(const WindowConfig& config) {
    auto window = std::make_unique<WindowWindows>(config);
    if (!window->IsOpen()) {
        return nullptr;
    }
    return window;
}

} // namespace ToyFrameV

#endif // PLATFORM_WINDOWS
