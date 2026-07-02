#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>

#include "Utils/Types.hpp"
#include "Utils/WinTypes.hpp"

namespace Flame {
  struct RenderContext final {
    RenderContext();
    ~RenderContext();

    bool Initialize(HWND handle, u32 width, u32 height);
    void Cleanup();

  private:
    bool CreateCommandQueue();
    bool CreateSwapChain(HWND handle, u32 width, u32 height);
    bool CreateDescriptorHeaps();
    bool CreateBackBuffers();
    bool CreateFence();

  private:
    static constexpr u32 kBufferCount = 2;

    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12Resource> m_backBuffers[kBufferCount];

    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    u64 m_fenceValue = 0;

    u32 m_currentBufferIndex = 0;
  };
}
