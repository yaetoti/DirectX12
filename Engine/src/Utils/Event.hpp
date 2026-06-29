#pragma once
#include <type_traits>

namespace Flame {
  struct BaseEvent {
    virtual ~BaseEvent() = default;

    void Cancel() const {
      m_isCancelled = true;
    }

    bool IsCancelled() const {
      return m_isCancelled;
    }

  protected:
    BaseEvent():
    m_isCancelled(false) {
    }

  private:
    mutable bool m_isCancelled;
  };

  template <typename TType>
  requires std::is_enum_v<TType>
  struct Event : BaseEvent {
    virtual ~Event() = default;

    TType GetType() const {
      return m_type;
    }

    template <typename T>
    requires std::derived_from<T, Event>
    T* As() {
      return static_cast<T*>(this);
    }

    template <typename T>
    requires std::derived_from<T, Event>
    const T* As() const {
      return static_cast<const T*>(this);
    }

  protected:
    Event(TType type):
    m_type(type) {
    }

  private:
    TType m_type;
  };
}
