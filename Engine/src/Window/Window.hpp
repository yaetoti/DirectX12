#pragma once

#include <string>
#include <Windows.h>

#include "InputSystem.hpp"
#include "Events/WindowEvent.hpp"
#include "Utils/EventQueue.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  struct Window final {
    Window(std::wstring  title, u32 width, u32 height);
    ~Window();

    bool Initialize();
    void Cleanup();
    void Show();

    void PollEvents();

    const EventQueue<Event<WindowEventType>>& GetEventQueue() const;
    const InputSystem& GetInputSystem() const;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

  private:
    HWND m_handle;
    u32 m_width;
    u32 m_height;
    std::wstring m_title;
    bool m_isFullscreen;

    EventQueue<Event<WindowEventType>> m_eventQueue;
    InputSystem m_inputSystem;

    inline static const wchar_t* kClassName = L"Window";
  };
}
