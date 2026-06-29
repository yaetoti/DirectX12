#pragma once

#include "Utils/Event.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  enum class WindowEventType {
    KEY
  };

  struct KeyWindowEvent final : Event<WindowEventType> {
    KeyWindowEvent():
    Event(WindowEventType::KEY) {
    }
  };
}