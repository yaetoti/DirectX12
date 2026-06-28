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
  };
}
