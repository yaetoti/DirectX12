#include "Texture.hpp"

#include <algorithm>

namespace Flame {
  void Texture::Reset() {
    m_resource = nullptr;
    m_state = D3D12_RESOURCE_STATE_COMMON;
  }

  void Texture::Reset(ComPtr<ID3D12Resource>&& resource, D3D12_RESOURCE_STATES state) {
    this->m_resource = std::move(resource);
    this->m_state = state;
  }

  void Texture::Transition(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES newState) {
    if (m_state == newState) {
      return;
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = m_state;
    barrier.Transition.StateAfter = newState;

    commandList->ResourceBarrier(1, &barrier);
    m_state = newState;
  }

  ID3D12Resource* Texture::GetResource() const {
    return m_resource.Get();
  }

  D3D12_RESOURCE_STATES Texture::GetState() const {
    return m_state;
  }
}
