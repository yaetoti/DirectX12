#include "Application.hpp"

#include <iostream>

#include "Utils/Logger.hpp"

Application::Application() {
  m_window = std::make_unique<Flame::Window>(L"Flame 3.0", 800, 600);
  m_context = std::make_unique<Flame::RenderContext>();
}

Application::~Application() {
  Cleanup();
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

  if (!m_context->Initialize(m_window->GetHandle(), m_window->GetWidth(), m_window->GetHeight())) {
    return false;
  }

  m_window->GetEventQueue().Subscribe([this](const Flame::Event<Flame::WindowEventType>& event) {
    HandleWindowEvent(event);
  });

  m_window->Show();
  return true;
}

void Application::Cleanup() {
  m_context.reset();
  m_window.reset();
}

void Application::Update() {
  auto input = m_window->GetInputSystem();
  if (input.HasText()) {
    Flame::Logger::Log(L"{}", input.GetText());
  }
}

void Application::Render() {
  m_context->BeginFrame();

  m_context->BindBackBufferRT();
  m_context->ClearRT(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  m_context->ClearDepthStencil();

  m_context->EndFrame();
}

void Application::HandleWindowEvent(const Flame::Event<Flame::WindowEventType>& event) {
  if (event.GetType() == Flame::WindowEventType::Resize) {
    auto& e = dynamic_cast<const Flame::ResizeWindowEvent&>(event);
    m_context->Resize(e.width, e.height);
  }
}
