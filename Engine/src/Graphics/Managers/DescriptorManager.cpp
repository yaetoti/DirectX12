#include "DescriptorManager.hpp"

#include <stdexcept>
#include <string>

#include "Graphics/RenderContext.hpp"

namespace Flame {
  void DescriptorManager::Start() {
    assert(s_instance == nullptr);
    s_instance = new DescriptorManager();
  }

  void DescriptorManager::Shutdown() {
    assert(s_instance != nullptr);
    delete s_instance;
  }

  DescriptorManager* DescriptorManager::Get() {
    assert(s_instance != nullptr);
    return s_instance;
  }

  bool DescriptorManager::Initialize() {
    m_heapsRTV.clear();
    m_heapsDSV.clear();
    AddHeapRTV();
    AddHeapDSV();
    return true;
  }

  DescriptorHandle DescriptorManager::AllocateRTV(u32 amount) {
    assert(amount <= kHeapSize && "Unsupported allocation size");

    // Allocate in existing heaps
    for (auto& heap : m_heapsRTV) {
      auto result = heap->Allocate(amount);
      if (result) {
        return std::move(*result);
      }
    }

    // Allocate a new heap
    if (!AddHeapRTV()) {
      throw std::runtime_error("Cannot initialize new descriptor heap");
    }

    auto heap = m_heapsRTV.back().get();
    auto result = heap->Allocate(amount);
    if (result) {
      return std::move(*result);
    }

    throw std::runtime_error("Cannot allocate descriptor handles");
  }

  DescriptorHandle DescriptorManager::AllocateDSV(u32 amount) {
    assert(amount <= kHeapSize && "Unsupported allocation size");

    // Allocate in existing heaps
    for (auto& heap : m_heapsDSV) {
      auto result = heap->Allocate(amount);
      if (result) {
        return std::move(*result);
      }
    }

    // Allocate a new heap
    if (!AddHeapDSV()) {
      throw std::runtime_error("Cannot initialize new descriptor heap");
    }

    auto heap = m_heapsDSV.back().get();
    auto result = heap->Allocate(amount);
    if (result) {
      return std::move(*result);
    }

    throw std::runtime_error("Cannot allocate descriptor handles");
  }

  bool DescriptorManager::AddHeapRTV() {
    auto& heap = m_heapsRTV.emplace_back(std::make_unique<DescriptorHeap>());
    return heap->Initialize(kHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
  }

  bool DescriptorManager::AddHeapDSV() {
    auto& heap = m_heapsDSV.emplace_back(std::make_unique<DescriptorHeap>());
    return heap->Initialize(kHeapSize, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);
  }
}
