#include <locale>
#include <Windows.h>

#include "Application.hpp"
#include "Engine.hpp"
#include "Utils/Logger.hpp"

int wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  std::locale::global(std::locale("en-US.UTF8"));

  if (!Flame::Engine::Start()) {
    return 0;
  }

  {
    auto application = std::make_unique<Application>();
    application->RunMainLoop();
  }

  Flame::Engine::Shutdown();
  return 0;
}
