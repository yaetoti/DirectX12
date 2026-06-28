#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <source_location>

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

    template <typename... Args>
    static void Log(std::source_location location, LogLevel level, std::wformat_string<Args...> fmt, Args&&... args) {
      std::wstring message = std::vformat(fmt.get(), std::make_wformat_args(args...));
      std::wstring result = GetLocationPrefix(location) + GetLogPrefix(level) + message + L'\n';

      HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      if (hConsole != INVALID_HANDLE_VALUE) {
        std::string output = LogHelper::WideToUtf8(result);
        WriteFile(hConsole, output.c_str(), output.size(), nullptr, nullptr);
      }
    }

  private:
    static std::wstring GetLocationPrefix(std::source_location location) {
      std::string filename(location.file_name());
      std::wstring wFilename(filename.begin(), filename.end());
      u32 line = location.line();
      u32 column = location.column();
      std::wstring locationStr = std::format(L"{}({},{}): ", wFilename, line, column);
      return locationStr;
    }

    static std::wstring GetLogPrefix(LogLevel level) {
      auto now = std::chrono::system_clock::now();
      auto seconds = std::chrono::floor<std::chrono::seconds>(now);
      std::wstring timestamp = std::format(L"[{:%H:%M:%S}]", seconds);

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
