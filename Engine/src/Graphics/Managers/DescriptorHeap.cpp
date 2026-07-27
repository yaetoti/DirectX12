#include "DescriptorHeap.hpp"

#include "Graphics/DescriptorHandle.hpp"
#include "Utils/Logger.hpp"

namespace Flame {
  DescriptorHeap::operator bool() const {
    return m_heap.Get() != nullptr;
  }

  bool DescriptorHeap::Initialize(u32 capacity, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible) {
    Reset();

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = type;
    desc.NumDescriptors = capacity;
    desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    // Allocate heap
    HRESULT hr = RenderCore::Get()->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_heap.GetAddressOf()));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create a descriptor heap. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    m_cpuHandle = m_heap->GetCPUDescriptorHandleForHeapStart();
    if (isShaderVisible) {
      m_gpuHandle = m_heap->GetGPUDescriptorHandleForHeapStart();
    }

    // Fill data
    m_increment = RenderCore::Get()->GetDevice()->GetDescriptorHandleIncrementSize(type);
    m_capacity = capacity;
    m_type = type;
    m_isShaderVisible = isShaderVisible;
    m_blocks.emplace_back(Block::Type::FREE, 0, capacity);

    return true;
  }

  void DescriptorHeap::Reset() {
    m_heap.Reset();
    m_blocks.clear();
    m_allocatedBlocks.clear();
  }

  std::optional<DescriptorHandle> DescriptorHeap::Allocate(u32 amount) {
    // Free-list allocator

    std::list<Block>::iterator bestMatch = m_blocks.end();
    u32 bestDifference = std::numeric_limits<u32>::max();

    for (auto it = m_blocks.begin(); it != m_blocks.end(); ++it) {
      if (it->type != Block::Type::FREE || it->size < amount) {
        continue;
      }

      u32 difference = it->size - amount;
      if (difference < bestDifference) {
        bestMatch = it;
        bestDifference = difference;
        // Best match
        if (difference == 0) {
          break;
        }
      }
    }

    if (bestMatch == m_blocks.end()) {
      return std::nullopt;
    }

    // Insert allocated block
    auto allocated = bestMatch;
    if (bestMatch->size == amount) {
      // Give full block
      allocated->type = Block::Type::ALLOCATED;
    }
    else {
      // Insert allocated block before the remaining free block
      allocated = m_blocks.emplace(bestMatch, Block::Allocated(bestMatch->start, amount));
      bestMatch->start += amount;
      bestMatch->size -= amount;
    }

    // Add lookup entry
    m_allocatedBlocks[allocated->start] = allocated;

    // Return handle
    auto cpuHandle = m_cpuHandle;
    auto gpuHandle = m_gpuHandle;
    cpuHandle.ptr += allocated->start * m_increment;
    gpuHandle.ptr += allocated->start * m_increment;

    auto result = DescriptorHandle();
    result.Initialize(cpuHandle, gpuHandle, this, allocated->start, amount);
    return result;
  }

  void DescriptorHeap::Free(u32 index) {
    auto block = m_allocatedBlocks.find(index);
    if (block == m_allocatedBlocks.end()) {
      assert("Wrong free index");
      return;
    }

    // Mark block as free and remove lookup entry
    auto it = block->second;
    it->type = Block::Type::FREE;
    m_allocatedBlocks.erase(index);

    // Coalesce previous
    if (it != m_blocks.begin()) {
      auto prev = std::prev(it);

      if (prev->type == Block::Type::FREE) {
        prev->size += it->size;
        m_blocks.erase(it);
        it = prev;
      }
    }

    // Coalesce next
    if (it != std::prev(m_blocks.end())) {
      auto next = std::next(it);

      if (next->type == Block::Type::FREE) {
        it->size += next->size;
        m_blocks.erase(next);
      }
    }
  }

  u32 DescriptorHeap::GetIncrement() const {
    return m_increment;
  }

  bool DescriptorHeap::IsShaderVisible() const {
    return m_isShaderVisible;
  }

  D3D12_DESCRIPTOR_HEAP_TYPE DescriptorHeap::GetType() const {
    return m_type;
  }
}
