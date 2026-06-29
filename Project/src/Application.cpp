#include "Application.hpp"

#include <iostream>

Application::Application() {
  m_window = std::make_unique<Flame::Window>(L"Flame 3.0", 800, 600);
}

Application::~Application() {

}

void Application::RunMainLoop() {
  if (!Initialize()) {
    return;
  }

  // Main Loop
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

    m_window->PollEvents();

    Update();
    Render();
  }
}

bool Application::Initialize() {
  if (!m_window->Initialize()) {
    return false;
  }

  m_window->GetEventQueue().Subscribe([](auto& e) {
    if (e.GetType() == Flame::WindowEventType::KEY) {
      auto* keyEvent = e.As<Flame::KeyWindowEvent>();
      std::cout << "Key: " << std::hex << keyEvent->vkCode << std::endl;
    }
    if (e.GetType() == Flame::WindowEventType::TEXT) {
      auto* keyEvent = e.As<Flame::TextWindowEvent>();
      std::wcout << L"Text: " << keyEvent->symbol << std::endl;
    }
  });

  m_window->Show();
  return true;
}

void Application::Update() {

}

void Application::Render() {

}
