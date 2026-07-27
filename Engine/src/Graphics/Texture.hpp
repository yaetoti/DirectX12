#pragma once

#include <d3d12.h>
#include "Utils/WinTypes.hpp"

namespace Flame {
  struct Texture final {
    void Reset();
    void Reset(ComPtr<ID3D12Resource>&& resource, D3D12_RESOURCE_STATES state);
    void Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState);

    ID3D12Resource* GetResource() const;
    D3D12_RESOURCE_STATES GetState() const;

  private:
    ComPtr<ID3D12Resource> m_resource;
    D3D12_RESOURCE_STATES m_state = D3D12_RESOURCE_STATE_COMMON;
  };
}
