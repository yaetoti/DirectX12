#include "DescriptorHandle.hpp"

#include <cassert>

#include "Managers/DescriptorManager.hpp"

namespace Flame {
  DescriptorHandle::DescriptorHandle(DescriptorHandle&& other) noexcept:
  m_cpuHandle(other.m_cpuHandle),
  m_gpuHandle(other.m_gpuHandle),
  m_heap(other.m_heap),
  m_index(other.m_index),
  m_size(other.m_size) {
    // Nullify
    other.m_heap = nullptr;
  }

  DescriptorHandle& DescriptorHandle::operator=(DescriptorHandle&& other) noexcept {
    if (this == &other) {
      return *this;
    }

    // Reset
    Reset();

    // Move
    m_cpuHandle = other.m_cpuHandle;
    m_gpuHandle = other.m_gpuHandle;
    m_heap = other.m_heap;
    m_index = other.m_index;
    m_size = other.m_size;

    // Nullify
    other.m_heap = nullptr;

    return *this;
  }

  DescriptorHandle::operator bool() const {
    return m_heap != nullptr;
  }

  DescriptorHandle::~DescriptorHandle() {
    Reset();
  }

  void DescriptorHandle::Initialize(D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, DescriptorHeap* heap, u32 index, u32 size) {
    Reset();

    m_cpuHandle = cpuHandle;
    m_gpuHandle = gpuHandle;
    m_heap = heap;
    m_index = index;
    m_size = size;
  }

  void DescriptorHandle::Reset() {
    if (m_heap) {
      m_heap->Free(m_index);
      m_heap = nullptr;
    }
  }

  D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle::Cpu(u32 offset) const {
    assert(offset < m_size);
    auto result = m_cpuHandle;
    result.ptr += offset * m_heap->GetIncrement();
    return result;
  }

  D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHandle::Gpu(u32 offset) const {
    assert(offset < m_size);
    assert(m_heap->IsShaderVisible());
    auto result = m_gpuHandle;
    result.ptr += offset * m_heap->GetIncrement();
    return result;
  }

  void DescriptorHandle::CreateRTV(u32 offset, ID3D12Resource* resource) const {
    assert(m_heap->GetType() == D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    RenderCore::Get()->GetDevice()->CreateRenderTargetView(resource, nullptr, Cpu(offset));
  }

  void DescriptorHandle::CreateDSV(u32 offset, ID3D12Resource* resource) const {
    assert(m_heap->GetType() == D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
    RenderCore::Get()->GetDevice()->CreateDepthStencilView(resource, nullptr, Cpu(offset));
  }
}
