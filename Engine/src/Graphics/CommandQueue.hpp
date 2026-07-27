#pragma once

#include <d3d12.h>
#include "Utils/Types.hpp"
#include "Utils/WinTypes.hpp"

namespace Flame {
  struct CommandQueue final {
    ~CommandQueue();

    bool Initialize();
    void Reset();

    void Execute(ID3D12CommandList* commandList);

    u64 Signal();
    void WaitForFence(u64 fenceValue);
    void Flush();

    ID3D12CommandQueue* GetCommandQueue() const;

  private:
    bool CreateCommandQueue();
    bool CreateFence();

  private:
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12Fence> m_fence;
    HANDLE m_fenceEvent = nullptr;
    u64 m_nextFenceValue = 0;
  };
}
