#include "RenderCore.hpp"

#include <cassert>

namespace Flame {
  RenderCore::RenderCore() {
  }

  RenderCore::~RenderCore() {
    Cleanup();
  }

  void RenderCore::Start() {
    assert(s_instance == nullptr);
    s_instance = new RenderCore();
  }

  void RenderCore::Shutdown() {
    assert(s_instance != nullptr);
    delete s_instance;
  }

  RenderCore* RenderCore::Get() {
    assert(s_instance != nullptr);
    return s_instance;
  }

  bool RenderCore::Initialize() {
    return true;
  }

  void RenderCore::Cleanup() {
  }
}
