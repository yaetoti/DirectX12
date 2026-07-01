#pragma once
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>

#include "Utils/WinTypes.hpp"

namespace Flame {
  struct RenderCore final {
    static void Start();
    static void Shutdown();
    static RenderCore* Get();

    bool Initialize();
    void Cleanup();

  private:
    RenderCore();
    ~RenderCore();
    RenderCore(const RenderCore&) = delete;
    RenderCore& operator=(const RenderCore&) = delete;

  private:
    ComPtr<IDXGIFactory7> m_factory;
    ComPtr<IDXGIAdapter4> m_adapter;
    ComPtr<ID3D12Device9> m_device;
    ComPtr<ID3D12Debug1> m_debug;

    inline static RenderCore* s_instance = nullptr;
  };
}
