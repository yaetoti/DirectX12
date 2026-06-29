#pragma once
#include <array>
#include <DirectXMath.h>
#include <string>

#include "Events/WindowEvent.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  enum class KeyState {
    Up,
    Pressed,
    Held,
    Released
  };

  struct InputSystem final {
    void Update();
    void HandleEvent(const Event<WindowEventType>& event);

    bool WasKeyPressed(u32 vkCode) const;
    bool WasKeyReleased(u32 vkCode) const;
    bool IsKeyDown(u32 vkCode) const;
    bool IsKeyUp(u32 vkCode) const;
    bool HasText() const;
    const std::wstring& GetText() const;

  public:
    static constexpr u32 kKeyCount = 256;

  private:
    std::array<KeyState, kKeyCount> m_keyStates { KeyState::Up };
    bool m_leaveLastSymbol = false;
    std::wstring m_accumulatedText;
    std::wstring m_text;

    // TODO add wchar text collection
    // TODO leave high surrogate for the next tick
  };
}
