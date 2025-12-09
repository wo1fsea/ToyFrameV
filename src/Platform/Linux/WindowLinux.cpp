/**
 * @file WindowLinux.cpp
 * @brief Linux platform window implementation using X11
 */

#include "ToyFrameV/Window.h"
#include "ToyFrameV/Input.h"
#include "ToyFrameV/Platform.h"

#ifdef PLATFORM_LINUX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstring>
#include "ToyFrameV/Core/Log.h"

namespace ToyFrameV {

// Forward declaration
KeyCode X11KeyToKeyCode(KeySym keysym);

/**
 * @brief Linux-specific window implementation using X11
 */
class WindowLinux : public Window {
public:
    WindowLinux(const WindowConfig& config);
    ~WindowLinux() override;

    bool ProcessEvents() override;
    void Close() override;
    bool IsOpen() const override { return m_window != 0 && m_isOpen; }
    int GetWidth() const override { return m_width; }
    int GetHeight() const override { return m_height; }
    void SetTitle(const std::string& title) override;
    void SetSize(int width, int height) override;
    void SetPosition(int x, int y) override;
    void SetVisible(bool visible) override;
    void* GetNativeHandle() const override { return (void*)(uintptr_t)m_window; }

private:
    Display* m_display;
    ::Window m_window;
    Atom m_wmDeleteMessage;
    int m_width;
    int m_height;
    bool m_isOpen;
    bool m_isVisible;
    std::string m_title;
};

WindowLinux::WindowLinux(const WindowConfig& config)
    : m_display(nullptr)
    , m_window(0)
    , m_wmDeleteMessage(0)
    , m_width(config.width)
    , m_height(config.height)
    , m_isOpen(false)
    , m_isVisible(false)
    , m_title(config.title)
{
    // Open connection to X server
    m_display = XOpenDisplay(nullptr);
    if (!m_display) {
        TOYFRAMEV_LOG_ERROR("Failed to open X display");
        return;
    }

    int screen = DefaultScreen(m_display);
    ::Window root = RootWindow(m_display, screen);

    // Create window
    m_window = XCreateSimpleWindow(
        m_display, root,
        config.posX, config.posY,
        config.width, config.height,
        1,
        BlackPixel(m_display, screen),
        WhitePixel(m_display, screen)
    );

    if (!m_window) {
        TOYFRAMEV_LOG_ERROR("Failed to create X window");
        XCloseDisplay(m_display);
        m_display = nullptr;
        return;
    }

    // Set window title
    XStoreName(m_display, m_window, config.title.c_str());

    // Select input events
    XSelectInput(m_display, m_window,
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
        StructureNotifyMask);

    // Handle window close button
    m_wmDeleteMessage = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_display, m_window, &m_wmDeleteMessage, 1);

    m_isOpen = true;
}

WindowLinux::~WindowLinux() {
    Close();
}

bool WindowLinux::ProcessEvents() {
    if (!m_display || !m_window) {
        return false;
    }

    while (XPending(m_display)) {
        XEvent event;
        XNextEvent(m_display, &event);

        switch (event.type) {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == m_wmDeleteMessage) {
                    m_isOpen = false;
                    return false;
                }
                break;

            case ConfigureNotify:
                if (event.xconfigure.width != m_width || event.xconfigure.height != m_height) {
                    m_width = event.xconfigure.width;
                    m_height = event.xconfigure.height;
                }
                break;

            case KeyPress:
            case KeyRelease: {
                KeySym keysym = XLookupKeysym(&event.xkey, 0);
                KeyCode keyCode = X11KeyToKeyCode(keysym);
                
                if (event.type == KeyPress) {
                    Input::_SetKeyState(keyCode, true);
                } else {
                    Input::_SetKeyState(keyCode, false);
                }
                break;
            }

            case ButtonPress:
            case ButtonRelease: {
                int button = event.xbutton.button - 1; // X11 buttons are 1-based
                if (button >= 0 && button < 5) {
                    if (event.type == ButtonPress) {
                        Input::_SetMouseButtonState(static_cast<MouseButton>(button), true);
                    } else {
                        Input::_SetMouseButtonState(static_cast<MouseButton>(button), false);
                    }
                }
                break;
            }

            case MotionNotify:
                Input::_SetMousePosition(event.xmotion.x, event.xmotion.y);
                break;
        }
    }

    return m_isOpen;
}

void WindowLinux::Close() {
    if (m_window) {
        XDestroyWindow(m_display, m_window);
        m_window = 0;
    }
    if (m_display) {
        XCloseDisplay(m_display);
        m_display = nullptr;
    }
    m_isOpen = false;
}

void WindowLinux::SetTitle(const std::string& title) {
    m_title = title;
    if (m_display && m_window) {
        XStoreName(m_display, m_window, title.c_str());
    }
}

void WindowLinux::SetSize(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_display && m_window) {
        XResizeWindow(m_display, m_window, width, height);
    }
}

void WindowLinux::SetPosition(int x, int y) {
    if (m_display && m_window) {
        XMoveWindow(m_display, m_window, x, y);
    }
}

void WindowLinux::SetVisible(bool visible) {
    if (m_display && m_window) {
        if (visible && !m_isVisible) {
            XMapWindow(m_display, m_window);
            m_isVisible = true;
        } else if (!visible && m_isVisible) {
            XUnmapWindow(m_display, m_window);
            m_isVisible = false;
        }
    }
}

// Factory function
std::unique_ptr<Window> Window::Create(const WindowConfig& config) {
    return std::make_unique<WindowLinux>(config);
}

} // namespace ToyFrameV

#endif // PLATFORM_LINUX
