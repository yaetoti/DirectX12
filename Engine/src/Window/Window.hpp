#pragma once

#include <string>
#include <Windows.h>

#include "InputSystem.hpp"
#include "Events/WindowEvent.hpp"
#include "Utils/EventQueue.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  enum class WindowState {
    Normal,
    Minimized,
    Maximized,
    Fullscreen,
  };

  struct Window final {
    Window(std::wstring  title, u32 width, u32 height);
    ~Window();

    bool Initialize();
    void Cleanup();
    void Show();

    void PollEvents();

    HWND GetHandle() const;
    u32 GetWidth() const;
    u32 GetHeight() const;
    const std::wstring& GetTitle() const;
    WindowState GetState() const;
    const EventQueue<Event<WindowEventType>>& GetEventQueue() const;
    const InputSystem& GetInputSystem() const;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  private:
    HWND m_handle;
    u32 m_width;
    u32 m_height;
    std::wstring m_title;
    WindowState m_state;

    EventQueue<Event<WindowEventType>> m_eventQueue;
    InputSystem m_inputSystem;

    inline static const wchar_t* kClassName = L"Window";
  };
}
