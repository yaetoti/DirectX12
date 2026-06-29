#pragma once
#include <memory>

#include "Window/Window.hpp"

struct Application final {
  Application();
  ~Application();

  void RunMainLoop();

private:
  bool Initialize();
  void Update();
  void Render();

private:
  std::unique_ptr<Flame::Window> m_window;
};
