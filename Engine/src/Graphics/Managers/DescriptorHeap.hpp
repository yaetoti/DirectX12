#pragma once
#include <d3d12.h>
#include <optional>
#include <list>
#include <unordered_map>

#include "Graphics/RenderCore.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  struct DescriptorHandle;

  struct DescriptorHeap final {
    operator bool() const;

    bool Initialize(u32 capacity, D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible);
    void Reset();

    std::optional<DescriptorHandle> Allocate(u32 amount);
    void Free(u32 index);

    u32 GetIncrement() const;
    bool IsShaderVisible() const;
    D3D12_DESCRIPTOR_HEAP_TYPE GetType() const;

  private:
    struct Block final {
      enum Type {
        FREE,
        ALLOCATED,
      };

      static Block Free(u32 start, u32 size) {
        return Block {
          .type = FREE,
          .start = start,
          .size = size
        };
      }

      static Block Allocated(u32 start, u32 size) {
        return Block {
          .type = ALLOCATED,
          .start = start,
          .size = size
        };
      }

      Type type;
      u32 start;
      u32 size;
    };

    ComPtr<ID3D12DescriptorHeap> m_heap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle = {}; // Can be garbage if not shader visible
    D3D12_DESCRIPTOR_HEAP_TYPE m_type = {};

    std::list<Block> m_blocks;
    std::unordered_map<u32, std::list<Block>::iterator> m_allocatedBlocks;
    u32 m_increment = 0;
    u32 m_capacity = 0;
    bool m_isShaderVisible = false;
  };
}

