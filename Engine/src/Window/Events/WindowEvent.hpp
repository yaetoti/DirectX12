#pragma once

#include "Utils/Event.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  enum class WindowEventType {
    Key,
    Text,
    Resize,
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

  struct ResizeWindowEvent final : Event<WindowEventType> {
    explicit ResizeWindowEvent(u32 width, u32 height):
    Event(WindowEventType::Resize),
    width(width),
    height(height) {}

    u32 width;
    u32 height;
  };
}