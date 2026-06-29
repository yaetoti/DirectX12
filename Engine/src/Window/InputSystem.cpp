#include "InputSystem.hpp"

#include <Windows.h>
#include <cassert>

#include "Events/WindowEvent.hpp"

namespace Flame {
  void InputSystem::Update() {
    // Update key states
    for (u32 i = 0; i < kKeyCount; ++i) {
      auto state = m_keyStates[i];
      if (state == KeyState::Pressed) m_keyStates[i] = KeyState::Held;
      else if (state == KeyState::Released) m_keyStates[i] = KeyState::Up;
    }

    // Update text
    if (m_leaveLastSymbol) {
      m_text = m_accumulatedText.substr(0, m_accumulatedText.size() - 1);
      m_accumulatedText = m_accumulatedText.substr(m_accumulatedText.size() - 1);
    }
    else {
      m_text = m_accumulatedText;
      m_accumulatedText.clear();
    }
  }

  void InputSystem::HandleEvent(const Event<WindowEventType>& event) {
    if (event.GetType() == WindowEventType::Key) {
      auto* e = event.As<KeyWindowEvent>();
      KeyState state = e->type == KeyWindowEvent::Type::Pressed ? KeyState::Pressed : KeyState::Released;
      m_keyStates[e->vkCode] = state;
      if (e->vkCode == VK_LSHIFT || e->vkCode == VK_RSHIFT) m_keyStates[VK_SHIFT] = state;
      if (e->vkCode == VK_LMENU || e->vkCode == VK_RMENU) m_keyStates[VK_MENU] = state;
      if (e->vkCode == VK_LCONTROL || e->vkCode == VK_RCONTROL) m_keyStates[VK_CONTROL] = state;
      return;
    }

    if (event.GetType() == WindowEventType::Text) {
      auto* e = event.As<TextWindowEvent>();
      wchar_t symbol = e->symbol;
      m_accumulatedText += symbol;
      m_leaveLastSymbol = IS_HIGH_SURROGATE(symbol);
      return;
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

  bool InputSystem::HasText() const {
    return !m_text.empty();
  }

  const std::wstring& InputSystem::GetText() const {
    return m_text;
  }
}
