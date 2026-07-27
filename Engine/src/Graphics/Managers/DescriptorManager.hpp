#pragma once
#include <d3d12.h>
#include <vector>

#include "DescriptorHeap.hpp"
#include "Graphics/DescriptorHandle.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  struct DescriptorManager final {
    static void Start();
    static void Shutdown();
    static DescriptorManager* Get();

    bool Initialize();

    DescriptorHandle AllocateRTV(u32 amount);
    DescriptorHandle AllocateDSV(u32 amount);

  private:
    bool AddHeapRTV();
    bool AddHeapDSV();

  private:
    std::vector<std::unique_ptr<DescriptorHeap>> m_heapsRTV;
    std::vector<std::unique_ptr<DescriptorHeap>> m_heapsDSV;

    inline static DescriptorManager* s_instance = nullptr;
    static constexpr u32 kHeapSize = 1024;
  };
}
