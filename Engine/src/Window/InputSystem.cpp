#include "InputSystem.hpp"

#include <cassert>

#include "events/WindowEvent.hpp"

namespace Flame {
  void InputSystem::HandleEvent(const Event<WindowEventType>& e) {

  }

  bool InputSystem::WasKeyPressed(u32 vkCode) const {
    assert(vkCode < kKeyCount);
    return m_keyStates[vkCode] == KeyState::Pressed;
  }

  bool InputSystem::WasKeyReleased(u32 vkCode) const {
    assert(vkCode < kKeyCount);
    return m_keyStates[vkCode] == KeyState::Released;
  }

  bool InputSystem::IsKeyDown(u32 vkCode) const {
    assert(vkCode < kKeyCount);
    return m_keyStates[vkCode] == KeyState::Pressed || m_keyStates[vkCode] == KeyState::Held;
  }

  bool InputSystem::IsKeyUp(u32 vkCode) const {
    assert(vkCode < kKeyCount);
    return m_keyStates[vkCode] == KeyState::Up || m_keyStates[vkCode] == KeyState::Released;
  }
}
