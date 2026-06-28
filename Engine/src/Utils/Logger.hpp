#pragma once
#include <chrono>
#include <format>
#include <iostream>

#include "LogHelper.hpp"
#include "Window/ClassManager.hpp"

namespace Flame {
  enum class LogLevel {
    Info,
    Warning,
    Error,
  };

  struct Logger final {
    template <typename... Args>
    static void Log(LogLevel level, std::wformat_string<Args...> fmt, Args&&... args) {
      std::wstring message = std::vformat(fmt.get(), std::make_wformat_args(args...));
      std::wstring result = GetLogPrefix(level) + message + L'\n';

      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsole != INVALID_HANDLE_VALUE) {
        std::string output = LogHelper::WideToUtf8(result);
        WriteFile(hConsole, output.c_str(), output.size(), nullptr, nullptr);
      }
    }

  private:
    static std::wstring GetLogPrefix(LogLevel level) {
      auto now = std::chrono::system_clock::now();
      std::wstring timestamp = std::format(L"[{:%T}]", now);

      switch (level) {
        case LogLevel::Info:
          return timestamp + L"[INFO]: ";
        case LogLevel::Warning:
          return timestamp + L"[WARNING]: ";
        case LogLevel::Error:
          return timestamp + L"[ERROR]: ";
        default:
          return timestamp + L"[LOG]";
      }
    }
  };
}
