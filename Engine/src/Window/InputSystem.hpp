#pragma once
#include <array>
#include <DirectXMath.h>

#include "events/WindowEvent.hpp"
#include "glm/vec2.hpp"
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
    void PostUpdate();
    void HandleEvent(const Event<WindowEventType>& event);

    bool WasKeyPressed(u32 vkCode) const;
    bool WasKeyReleased(u32 vkCode) const;
    bool IsKeyDown(u32 vkCode) const;
    bool IsKeyUp(u32 vkCode) const;

  public:
    static constexpr u32 kKeyCount = 256;

  private:
    std::array<KeyState, kKeyCount> m_keyStates { KeyState::Up };
    // TODO add wchar text collection
    // TODO leave high surrogate for the next tick
  };
}
