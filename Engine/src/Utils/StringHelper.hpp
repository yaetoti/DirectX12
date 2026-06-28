#pragma once
#include <Windows.h>
#include <string>

namespace Flame {
  struct StringHelper final {
    static std::string WideToUtf8(const std::wstring& s) {
      if (s.empty()) {
        return {};
      }

      int requiredSize = WideCharToMultiByte(CP_UTF8, 0, &s[0], (int)s.size(), nullptr, 0, nullptr, nullptr);
      std::string result(requiredSize, 0);
      WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), requiredSize, nullptr, nullptr);

      return result;
    }

    static std::wstring Utf8ToWide(const std::string& s) {
      if (s.empty()) {
        return {};
      }

      int requiredSize = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
      std::wstring result(requiredSize, 0);
      MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), requiredSize);

      return result;
    }
  };
}
