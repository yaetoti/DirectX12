#pragma once
#include <functional>
#include <memory>
#include <queue>

#include "Event.hpp"

namespace Flame {
  template <typename TEvent>
  requires std::derived_from<TEvent, BaseEvent>
  struct EventQueue final {
    using Handler = std::function<void(const TEvent&)>;

    auto Subscribe(Handler handler) {
      return m_handlers.emplace_back(handler);
    }

    void Unsubscribe(Handler handler) {
      m_handlers.erase(handler);
    }

    void Add(std::unique_ptr<TEvent> e) {
      m_queue.emplace(std::move(e));
    }

    void Clear() {
      m_queue.clear();
    }

    void Flush() {
      while (!m_queue.empty()) {
        const TEvent& event = *m_queue.front();
        for (const auto& handler : m_handlers) {
          if (event.IsCancelled()) {
            break;
          }

          handler(event);
        }

        m_queue.pop();
      }
    }

    const std::queue<std::unique_ptr<TEvent>>& GetEvents() const {
      return m_queue;
    }

  private:
    std::queue<std::unique_ptr<TEvent>> m_queue;
    std::list<Handler> m_handlers;
  };
}
