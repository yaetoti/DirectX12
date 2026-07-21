#pragma once
#include <d3d12.h>
#include <dxgi1_5.h>

#include "glm/vec4.hpp"
#include "Utils/Types.hpp"
#include "Utils/WinTypes.hpp"

namespace Flame {
  struct RenderContext final {
    RenderContext();
    ~RenderContext();

    bool Initialize(HWND handle, u32 width, u32 height);
    void Cleanup();

    void OnResize(u32 width, u32 height);

    void BeginFrame();
    void EndFrame();

    void BindBackBufferRT();
    void ClearRT(glm::vec4 color);

    ID3D12CommandList* GetCommandList() const;

  private:
    void WaitForAll();

    bool CreateCommandQueue();
    bool CreateSwapChain(HWND handle, u32 width, u32 height);
    bool CreateDescriptorHeaps();
    bool CreateBackBuffers();
    bool CreateDepthStencilBuffer(u32 width, u32 height);
    bool CreateFence();
    bool CreateCommandAllocators();
    bool CreateCommandList();

    bool HandleResize();

  private:
    static constexpr u32 kBufferCount = 2;

    // Devices
    ComPtr<IDXGISwapChain4> m_swapChain;
    ComPtr<ID3D12CommandQueue> m_commandQueue;

    // Back Buffers
    ComPtr<ID3D12Resource> m_backBuffers[kBufferCount];
    D3D12_RESOURCE_STATES m_backBufferStates[kBufferCount] = { };

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHeapHandles[kBufferCount] = { };

    // Depth Buffer
    ComPtr<ID3D12Resource> m_depthBuffer;
    ComPtr<ID3D12DescriptorHeap> m_depthHeap;

    // Command Queue
    ComPtr<ID3D12CommandAllocator> m_allocators[kBufferCount];
    ComPtr<ID3D12GraphicsCommandList6> m_commandList;

    // Synchronization
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    u64 m_fenceValues[kBufferCount] = { };
    u64 m_fenceCounter = 0;

    // Pipeline state
    ComPtr<ID3D12PipelineState> m_pipelineState;

    // Other
    u32 m_currentBufferIndex = 0;
    u32 m_rtvDescriptorSize = 0;

    // Size
    u32 m_width = 0;
    u32 m_height = 0;
    bool m_wasResized = false;
  };
}
