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

    ID3D12Debug1* GetDebug();
    IDXGIFactory7* GetFactory();
    IDXGIAdapter4* GetAdapter();
    ID3D12Device9* GetDevice();

  private:
    RenderCore();
    ~RenderCore();
    RenderCore(const RenderCore&) = delete;
    RenderCore& operator=(const RenderCore&) = delete;

    bool CreateDebugLayer();
    bool CreateFactory();
    bool SelectHardwareAdapter();
    bool CreateDevice();

  private:
    ComPtr<ID3D12Debug1> m_debug;
    ComPtr<IDXGIFactory7> m_factory;
    ComPtr<IDXGIAdapter4> m_adapter;
    ComPtr<ID3D12Device9> m_device;

    inline static RenderCore* s_instance = nullptr;
  };
}
