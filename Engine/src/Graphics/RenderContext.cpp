#include "RenderContext.hpp"

#include <source_location>

#include "RenderCore.hpp"
#include "Utils/Logger.hpp"
#include "Utils/LogHelper.hpp"

namespace Flame {
  RenderContext::RenderContext() {
  }

  RenderContext::~RenderContext() {
    Cleanup();
  }

  bool RenderContext::Initialize(HWND handle, u32 width, u32 height) {
    if (!CreateCommandQueue()) {
      return false;
    }

    if (!CreateSwapChain(handle, width, height)) {
      return false;
    }

    if (!CreateDescriptorHeaps()) {
      return false;
    }

    if (!CreateBackBuffers()) {
      return false;
    }

    if (!CreateFence()) {
      return false;
    }

    if (!CreateCommandAllocators()) {
      return false;
    }

    if (!CreateCommandList()) {
      return false;
    }

    return true;
  }

  void RenderContext::Cleanup() {
    if (m_commandQueue) {
      WaitForFence();
    }

    if (m_fenceEvent) {
      CloseHandle(m_fenceEvent);
      m_fenceEvent = nullptr;
    }

    m_commandList.Reset();

    for (auto& allocator : m_allocators) {
      allocator.Reset();
    }

    for (auto& buffer : m_backBuffers) {
      buffer.Reset();
    }

    m_fence.Reset();
    m_rtvHeap.Reset();
    m_swapChain.Reset();
    m_commandQueue.Reset();
  }

  void RenderContext::BeginFrame() {
    HRESULT hr = S_OK;
    u64 completedFenceValue = m_fence->GetCompletedValue();
    if (completedFenceValue < m_fenceValues[m_currentBufferIndex]) {
      m_fence->SetEventOnCompletion(m_fenceValues[m_currentBufferIndex], m_fenceEvent);
      WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    m_allocators[m_currentBufferIndex]->Reset();
    hr = m_commandList->Reset(m_allocators[m_currentBufferIndex].Get(), nullptr);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to reset command list. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_backBuffers[m_currentBufferIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
    rtvHandle.ptr += (m_currentBufferIndex * m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    const float clearColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
  }

  void RenderContext::EndFrame() {
    HRESULT hr = S_OK;
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_backBuffers[m_currentBufferIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_commandList->ResourceBarrier(1, &barrier);
    m_commandList->Close();

    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);

    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to present swap chain. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }

    m_fenceCounter += 1;
    m_fenceValues[m_currentBufferIndex] = m_fenceCounter;
    hr = m_commandQueue->Signal(m_fence.Get(), m_fenceCounter);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to signal fence. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }

    m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
  }

  void RenderContext::WaitForFence() {
    m_fenceCounter += 1;
    m_commandQueue->Signal(m_fence.Get(), m_fenceCounter);
    m_fence->SetEventOnCompletion(m_fenceCounter, m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);
  }

  bool RenderContext::CreateCommandQueue() {
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    // Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    hr = device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create command queue. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    return true;
  }

  bool RenderContext::CreateSwapChain(HWND handle, u32 width, u32 height) {
    HRESULT hr = S_OK;
    auto* factory = RenderCore::Get()->GetFactory();

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = kBufferCount;
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    ComPtr<IDXGISwapChain1> swapChain;
    hr = factory->CreateSwapChainForHwnd(m_commandQueue.Get(), handle, &swapChainDesc, nullptr, nullptr, &swapChain);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create swap chain. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    swapChain.As(&m_swapChain);
    m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
    return true;
  }

  bool RenderContext::CreateDescriptorHeaps() {
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = kBufferCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create descriptor heap. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    return true;
  }

  bool RenderContext::CreateBackBuffers() {
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (u32 i = 0; i < kBufferCount; ++i) {
      hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to get buffer. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        return false;
      }

      device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
      rtvHandle.ptr += m_rtvDescriptorSize;
    }

    return true;
  }

  bool RenderContext::CreateFence() {
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create fence. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!m_fenceEvent) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create fence event");
      return false;
    }

    return true;
  }

  bool RenderContext::CreateCommandAllocators() {
    auto* device = RenderCore::Get()->GetDevice();
    for (u32 i = 0; i < kBufferCount; ++i) {
      HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_allocators[i]));
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create command allocator. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        return false;
      }
    }

    return true;
  }

  bool RenderContext::CreateCommandList() {
    auto* device = RenderCore::Get()->GetDevice();
    HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_allocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create command list. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    hr = m_commandList->Close();
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to close command list. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    return true;
  }
}
