#include <Windows.h>

#include "Window/Window.hpp"
#include "Utils/Logger.hpp"

int wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  // Start
  Flame::ClassManager::Start();
  Flame::ClassManager::Get()->Initialize();


  // Application
  auto window = std::make_unique<Flame::Window>(L"Flame 3.0", 800, 600);
  auto window2 = std::make_unique<Flame::Window>(L"Flame 3.0", 800, 600);
  if (!window->Initialize()) {
    return 0;
  }

  if (!window2->Initialize()) {
    return 0;
  }

  bool isRunning = true;
  MSG msg;

  while (isRunning) {
    // Event
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        isRunning = false;
      }

      TranslateMessage(&msg);
      DispatchMessageW(&msg);
    }

    // Update
    // Render
  }

  // Stop
  Flame::ClassManager::Shutdown();
  return 0;
}
