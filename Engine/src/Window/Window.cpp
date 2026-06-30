#include "Window.hpp"

#include <iostream>
#include <ostream>
#include <utility>

#include "ClassManager.hpp"

namespace Flame {
  Window::Window(std::wstring title, u32 width, u32 height):
  m_handle(nullptr),
  m_width(width),
  m_height(height),
  m_title(std::move(title)),
  m_state(WindowState::Normal) {
  }

  Window::~Window() {
    Cleanup();
  }

  bool Window::Initialize() {
    // Register class
    if (!ClassManager::Get()->Has(kClassName)) {
      WNDCLASSEXW wc = { };
      wc.cbSize = sizeof(WNDCLASSEXW);
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;
      wc.style = CS_HREDRAW | CS_VREDRAW;
      wc.lpfnWndProc = WndProc;
      wc.hInstance = GetModuleHandleW(nullptr);
      wc.lpszClassName = kClassName;
      wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
      wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
      wc.hIconSm = LoadIconW(nullptr, IDI_APPLICATION);
      wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
      wc.lpszMenuName = nullptr;

      if (!ClassManager::Get()->AddEx(&wc)) {
        return false;
      }
    }

    // TODO window state handling: minimized, maximized, fullscreen, normal
    // Define style
    DWORD styleEx = 0;
    DWORD style = WS_OVERLAPPEDWINDOW;

    // Adjust window size
    RECT rect = { 0, 0, (LONG)m_width, (LONG)m_height };
    AdjustWindowRect(&rect, style, FALSE);

    // Register window
    m_handle = CreateWindowExW(
      styleEx,
      kClassName,
      m_title.data(),
      style,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      rect.right - rect.left,
      rect.bottom - rect.top,
      nullptr,
      nullptr,
      GetModuleHandleW(nullptr),
      this
    );

    if (!m_handle) {
      return false;
    }

    m_eventQueue.Subscribe([&](auto& e) { m_inputSystem.HandleEvent(e); });
    return true;
  }

  void Window::Cleanup() {
    if (m_handle) {
      DestroyWindow(m_handle);
    }
  }

  void Window::Show() {
    ShowWindow(m_handle, SW_SHOW);
  }

  void Window::PollEvents() {
    m_inputSystem.Update();
    // TODO consume events here, flush later

    m_eventQueue.Flush();
  }

  const EventQueue<Event<WindowEventType>>& Window::GetEventQueue() const {
    return m_eventQueue;
  }

  const InputSystem& Window::GetInputSystem() const {
    return m_inputSystem;
  }

  LRESULT Window::WndProc(HWND handle, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_CREATE) {
      auto data = (CREATESTRUCTW*)lParam;
      SetWindowLongPtrW(handle, GWLP_USERDATA, (LONG_PTR)data->lpCreateParams);
      return 0;
    }

    if (msg == WM_DESTROY) {
      PostQuitMessage(0);
      return 0;
    }

    auto window = (Window*)GetWindowLongPtrW(handle, GWLP_USERDATA);
    if (!window) {
      return DefWindowProcW(handle, msg, wParam, lParam);
    }

    switch (msg) {
      case WM_KEYDOWN:
      case WM_KEYUP:
      case WM_SYSKEYDOWN:
      case WM_SYSKEYUP: {
        u64 vkCode = wParam;
        u64 scanCode = (lParam >> 16) & 0xFF;
        bool isExtended = (lParam & (1 << 24)) != 0;
        bool previousState = lParam & (1 << 30);
        KeyWindowEvent::Type type;

        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
          // Skip repetition
          if (previousState) {
            return 0;
          }

          type = KeyWindowEvent::Type::Pressed;
        }
        else {
          // Skip repetition
          if (!previousState) {
            return 0;
          }

          type = KeyWindowEvent::Type::Released;
        }

        // Handle left-right
        if (vkCode == VK_CONTROL) vkCode = isExtended ? VK_LCONTROL : VK_RCONTROL;
        if (vkCode == VK_MENU) vkCode = isExtended ? VK_LMENU : VK_RMENU;
        if (vkCode == VK_SHIFT) {
          vkCode = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);
        }

        window->m_eventQueue.Add(std::make_unique<KeyWindowEvent>(type, vkCode, scanCode));
        return 0;
      }
      case WM_CHAR: {
        wchar_t symbol = (wchar_t)wParam;
        // Skip control characters
        if (iswcntrl(symbol) && !iswspace(symbol)) {
          return 0;
        }

        window->m_eventQueue.Add(std::make_unique<TextWindowEvent>(symbol));
        return 0;
      }
      case WM_SIZE: {
        // TODO handle minimize maximize fullscreen
        u32 width = LOWORD(lParam);
        u32 height = HIWORD(lParam);
        window->m_width = width;
        window->m_height = height;
        window->m_eventQueue.Add(std::make_unique<ResizeWindowEvent>(width, height));
        return 0;
      }
    }

    return DefWindowProcW(handle, msg, wParam, lParam);

    // TODO resize
    // TODO hide show
    // TODO focus gain lost
    // TODO mouse position
    // TODO mouse scroll
  }
}
