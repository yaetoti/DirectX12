#pragma once
#include <chrono>
#include <format>
#include <iostream>
#include <source_location>

#include "LogHelper.hpp"
#include "StringHelper.hpp"

namespace Flame {
  enum class LogLevel {
    Info,
    Warning,
    Error,
  };

  struct Logger final {
    // wchar_t
    template <typename... Args>
    static void Log(std::wformat_string<Args...> fmt, Args&&... args) {
      std::wstring message = std::vformat(fmt.get(), std::make_wformat_args(args...));
      std::string output = StringHelper::WideToUtf8(message);
      std::cout << output;
      std::cout.flush();
    }

    template <typename... Args>
    static void Log(LogLevel level, std::wformat_string<Args...> fmt, Args&&... args) {
      std::wstring message = std::vformat(fmt.get(), std::make_wformat_args(args...));
      std::wstring result = GetLogPrefixW(level) + message;
      std::string output = StringHelper::WideToUtf8(result);
      std::cout << output << std::endl;
    }

    template <typename... Args>
    static void Log(std::source_location location, LogLevel level, std::wformat_string<Args...> fmt, Args&&... args) {
      std::wstring message = std::vformat(fmt.get(), std::make_wformat_args(args...));
      std::wstring result = GetLocationPrefixW(location) + GetLogPrefixW(level) + message;
      std::string output = StringHelper::WideToUtf8(result);
      std::cout << output << std::endl;
    }

    // char
    template <typename... Args>
    static void Log(std::format_string<Args...> fmt, Args&&... args) {
      std::string message = std::vformat(fmt.get(), std::make_format_args(args...));
      std::string result = message;
      std::cout << result << std::endl;
    }

    template <typename... Args>
    static void Log(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
      std::string message = std::vformat(fmt.get(), std::make_format_args(args...));
      std::string result = StringHelper::WideToUtf8(GetLogPrefixW(level)) + message;
      std::cout << result << std::endl;
    }

    template <typename... Args>
    static void Log(std::source_location location, LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
      std::string message = std::vformat(fmt.get(), std::make_format_args(args...));
      std::string result = StringHelper::WideToUtf8(GetLocationPrefixW(location) + GetLogPrefixW(level)) + message;
      std::cout << result << std::endl;
    }

  private:
    static std::wstring GetLocationPrefixW(std::source_location location) {
      std::string filename(location.file_name());
      std::wstring wFilename(filename.begin(), filename.end());
      u32 line = location.line();
      u32 column = location.column();
      std::wstring locationStr = std::format(L"{}({},{}): ", wFilename, line, column);
      return locationStr;
    }

    static std::wstring GetLogPrefixW(LogLevel level) {
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
