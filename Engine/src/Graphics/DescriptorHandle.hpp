#pragma once
#include <d3d12.h>

#include "Utils/Types.hpp"

namespace Flame {
  struct DescriptorHeap;

  struct DescriptorHandle final {
    DescriptorHandle() = default;
    ~DescriptorHandle();
    DescriptorHandle(const DescriptorHandle& other) = delete;
    DescriptorHandle(DescriptorHandle&& other) noexcept;
    DescriptorHandle& operator=(const DescriptorHandle& other) = delete;
    DescriptorHandle& operator=(DescriptorHandle&& other) noexcept;

    operator bool() const;

    void Initialize(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, DescriptorHeap* heap, u32 index, u32 size);
    void Reset();
    void CreateRTV(u32 offset, ID3D12Resource* resource) const;
    void CreateDSV(u32 offset, ID3D12Resource* resource) const;

    D3D12_CPU_DESCRIPTOR_HANDLE Cpu(u32 offset) const;
    D3D12_GPU_DESCRIPTOR_HANDLE Gpu(u32 offset) const;

  private:
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle; // Can be garbage if not shader visible
    DescriptorHeap* m_heap = nullptr;
    u32 m_index;
    u32 m_size;
  };
}
