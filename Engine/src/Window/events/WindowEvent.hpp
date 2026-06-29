#pragma once

#include "Utils/Event.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  enum class WindowEventType {
    Key,
    Text,
  };

  struct KeyWindowEvent final : Event<WindowEventType> {
    enum class Type {
      Pressed,
      Released,
    };

    KeyWindowEvent(Type type, u64 vkCode, u64 scanCode):
    Event(WindowEventType::Key),
    type(type),
    vkCode(vkCode),
    scanCode(scanCode) {
    }

    Type type;
    u64 vkCode;
    u64 scanCode;
  };

  struct TextWindowEvent final : Event<WindowEventType> {
    explicit TextWindowEvent(wchar_t symbol):
    Event(WindowEventType::Text),
    symbol(symbol) {}

    wchar_t symbol;
  };
}