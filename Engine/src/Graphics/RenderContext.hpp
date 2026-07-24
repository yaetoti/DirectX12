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

    void Resize(u32 width, u32 height);

    void BeginFrame();
    void EndFrame();

    void BindBackBufferRT();
    void ClearRT(glm::vec4 color);
    void ClearDepthStencil();

    ID3D12CommandList* GetCommandList() const;

  private:
    bool CreateSwapChain(HWND handle, u32 width, u32 height);
    bool CreateCommandAllocators();
    bool CreateCommandList();

    bool CreateDescriptorHeaps();
    bool CreateBackBuffers();
    bool CreateDepthStencilBuffer(u32 width, u32 height);
    bool ThisShit();

    bool HandleResize();

  private:
    static constexpr u32 kBufferCount = 2;

    // Context
    ComPtr<IDXGISwapChain4> m_swapChain;

    // Commands
    ComPtr<ID3D12CommandAllocator> m_allocators[kBufferCount];
    ComPtr<ID3D12GraphicsCommandList6> m_commandList;

    // Back Buffers
    ComPtr<ID3D12Resource> m_backBuffers[kBufferCount];
    D3D12_RESOURCE_STATES m_backBufferStates[kBufferCount] = { };

    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHeapHandles[kBufferCount] = { };

    u64 m_fenceValues[kBufferCount] = { };
    u32 m_currentBufferIndex = 0;

    // Depth Buffer
    ComPtr<ID3D12Resource> m_depthBuffer;
    ComPtr<ID3D12DescriptorHeap> m_depthHeap;

    // Pipeline state
    ComPtr<ID3D12PipelineState> m_pipelineState;

    // Size
    u32 m_width = 0;
    u32 m_height = 0;
    bool m_wasResized = false;
  };
}
