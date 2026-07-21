#pragma once

#include <chrono>
#include "Logger.hpp"

namespace Flame {
    struct ScopedTimer final {
        ScopedTimer() {
            m_startTime = std::chrono::high_resolution_clock::now();
        }

        ~ScopedTimer() {
            m_endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(m_endTime - m_startTime).count();
            Logger::Log(L"Passed: {} ns.", duration);
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_endTime;
    };
}
