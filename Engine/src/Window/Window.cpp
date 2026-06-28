#include "Window.hpp"

#include <utility>

#include "ClassManager.hpp"

namespace Flame {
  Window::Window(std::wstring title, u32 width, u32 height):
  m_handle(nullptr),
  m_width(width),
  m_height(height),
  m_title(std::move(title)),
  m_isFullscreen(false) {
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

    // Define style
    DWORD styleEx = 0;
    DWORD style = m_isFullscreen ? WS_POPUP : WS_OVERLAPPEDWINDOW;

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

  LRESULT Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
  }
}
