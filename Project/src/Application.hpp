#pragma once
#include <memory>

#include "Graphics/RenderContext.hpp"
#include "Window/Window.hpp"

struct Application final {
  Application();
  ~Application();

  void RunMainLoop();

private:
  bool Initialize();
  void Cleanup();
  void Update();
  void Render();

private:
  std::unique_ptr<Flame::Window> m_window;
  std::unique_ptr<Flame::RenderContext> m_context;
};
