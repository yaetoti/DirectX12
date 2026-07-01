#pragma once
#include "Graphics/RenderCore.hpp"
#include "Window/ClassManager.hpp"

namespace Flame {
  struct Engine final {
    static bool Start() {
      ClassManager::Start();
      if (!ClassManager::Get()->Initialize()) {
        return false;
      }

      RenderCore::Start();
      if (!RenderCore::Get()->Initialize()) {
        return false;
      }

      return true;
    }

    static void Shutdown() {
      RenderCore::Shutdown();
      ClassManager::Shutdown();
    }
  };
}
