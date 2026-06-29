#include "InputSystem.hpp"

#include <Windows.h>
#include <cassert>

#include "events/WindowEvent.hpp"

namespace Flame {
  void InputSystem::Update() {
    // Update key states
    for (u32 i = 0; i < kKeyCount; ++i) {
      auto state = m_keyStates[i];
      if (state == KeyState::Pressed) m_keyStates[i] = KeyState::Held;
      else if (state == KeyState::Released) m_keyStates[i] = KeyState::Up;
    }
  }

  void InputSystem::PostUpdate() {

  }

  void InputSystem::HandleEvent(const Event<WindowEventType>& event) {
    if (event.GetType() == WindowEventType::Key) {
      auto* e = event.As<KeyWindowEvent>();
      KeyState state = e->type == KeyWindowEvent::Type::Pressed ? KeyState::Pressed : KeyState::Released;
      m_keyStates[e->vkCode] = state;
      if (e->vkCode == VK_LSHIFT || e->vkCode == VK_RSHIFT) m_keyStates[VK_SHIFT] = state;
      if (e->vkCode == VK_LMENU || e->vkCode == VK_RMENU) m_keyStates[VK_MENU] = state;
      if (e->vkCode == VK_LCONTROL || e->vkCode == VK_RCONTROL) m_keyStates[VK_CONTROL] = state;
    }
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
