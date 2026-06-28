#pragma once
#include "Window/ClassManager.hpp"
#include "Types.hpp"

namespace Flame {
  struct LogHelper final {
    static std::wstring GetWin32ErrorString(DWORD code) {
      LPWSTR messageBuffer = nullptr;
      u64 size = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        code,
        0,
        (LPWSTR)&messageBuffer,
        0,
        nullptr
      );

      if (size != 0 && messageBuffer[size - 1] == L'\n') {
        size--;
      }

      if (size != 0 && messageBuffer[size - 1] == L'\r') {
        size--;
      }

      std::wstring message(messageBuffer, size);
      LocalFree(messageBuffer);
      return message;
    }

    static std::string WideToUtf8(const std::wstring& s) {
      if (s.empty()) {
        return {};
      }

      int size_needed = WideCharToMultiByte(CP_UTF8, 0, &s[0], (int)s.size(), nullptr, 0, nullptr, nullptr);
      std::string result(size_needed, 0);
      WideCharToMultiByte(CP_UTF8, 0, &s[0], (int)s.size(), &result[0], size_needed, nullptr, nullptr);

      return result;
    }
  };
}
