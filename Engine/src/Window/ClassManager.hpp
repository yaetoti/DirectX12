#pragma once
#include <string>
#include <unordered_set>

#include "Window.hpp"

namespace Flame {
  struct ClassManager final {
    ~ClassManager();

    static void Start();
    static void Shutdown();
    static ClassManager* Get();

    bool Initialize();
    void Cleanup();

    bool Has(const std::wstring& name) const;
    bool AddEx(const WNDCLASSEXW* wndClass);
    void Remove(const std::wstring& className);

  private:
    std::unordered_set<std::wstring> m_classes;
  };
}
