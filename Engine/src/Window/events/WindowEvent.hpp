#pragma once

#include "Utils/Event.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  enum class WindowEventType {
    KEY,
    TEXT,
  };

  struct KeyWindowEvent final : Event<WindowEventType> {
    KeyWindowEvent(u64 vkCode, u64 scanCode):
    Event(WindowEventType::KEY),
    vkCode(vkCode),
    scanCode(scanCode) {
    }

    u64 vkCode;
    u64 scanCode;
  };

  struct TextWindowEvent final : Event<WindowEventType> {
    explicit TextWindowEvent(wchar_t symbol):
    Event(WindowEventType::TEXT),
    symbol(symbol) {}

    wchar_t symbol;
  };
}