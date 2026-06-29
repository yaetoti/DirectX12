#pragma once
#include "Window/ClassManager.hpp"

namespace Flame {
  struct Engine final {
    static bool Start() {
      ClassManager::Start();
      ClassManager::Get()->Initialize();

      return true;
    }

    static void Shutdown() {
      ClassManager::Shutdown();
    }
  };
}
