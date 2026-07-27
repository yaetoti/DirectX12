#pragma once
#include "Graphics/RenderCore.hpp"
#include "Graphics/Managers/DescriptorManager.hpp"
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

      DescriptorManager::Start();
      if (!DescriptorManager::Get()->Initialize()) {
        return false;
      }

      return true;
    }

    static void Shutdown() {
      DescriptorManager::Shutdown();
      RenderCore::Shutdown();
      ClassManager::Shutdown();
    }
  };
}
