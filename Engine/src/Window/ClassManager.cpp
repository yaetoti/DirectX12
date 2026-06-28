#include "ClassManager.hpp"

#include <cassert>

#include "Window.hpp"
#include "Utils/Logger.hpp"
#include "Utils/LogHelper.hpp"

namespace Flame {
  static ClassManager* s_instance = nullptr;

  ClassManager::~ClassManager() {
    Cleanup();
  }

  void ClassManager::Start() {
    assert(s_instance == nullptr);
    s_instance = new ClassManager();
  }

  void ClassManager::Shutdown() {
    assert(s_instance != nullptr);
    delete s_instance;
  }

  ClassManager* ClassManager::Get() {
    return s_instance;
  }

  void ClassManager::Initialize() {
  }

  void ClassManager::Cleanup() {
  }

  bool ClassManager::Has(const std::wstring& name) const {
    return m_classes.contains(name);
  }

  bool ClassManager::AddEx(const WNDCLASSEXW* wndClass) {
    if (!wndClass || !wndClass->lpszClassName) {
      Logger::Log(LogLevel::Error, L"Class pointer or name is null");
      return false;
    }

    if (m_classes.contains(wndClass->lpszClassName)) {
      Logger::Log(LogLevel::Error, L"Window class already exists");
      return false;
    }

    auto result = RegisterClassExW(wndClass);
    if (!result) {
      DWORD error = GetLastError();
      Logger::Log(LogLevel::Error, L"Failed to register window class '{}'", LogHelper::GetWin32ErrorString(error));
      return false;
    }

    m_classes.emplace(wndClass->lpszClassName);
    return true;
  }

  void ClassManager::Remove(const std::wstring& className) {
    m_classes.erase(className);
  }
}
