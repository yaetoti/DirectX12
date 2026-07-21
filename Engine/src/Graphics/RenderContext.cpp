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
    m_width = width;
    m_height = height;
    m_wasResized = false;

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

    if (!CreateDepthStencilBuffer(width, height)) {
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
      WaitForAll();
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

  void RenderContext::OnResize(u32 width, u32 height) {
    if (m_width == width && m_height == height) {
      return;
    }

    m_wasResized = true;
    m_width = width;
    m_height = height;
  }

  void RenderContext::BeginFrame() {
    HRESULT hr = S_OK;

    if (!HandleResize()) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to handle resize");
      return;
    }

    // Wait for frame
    u64 completedFenceValue = m_fence->GetCompletedValue();
    if (completedFenceValue < m_fenceValues[m_currentBufferIndex]) {
      m_fence->SetEventOnCompletion(m_fenceValues[m_currentBufferIndex], m_fenceEvent);
      WaitForSingleObject(m_fenceEvent, INFINITE);
    }

    // Reset allocator memory
    m_allocators[m_currentBufferIndex]->Reset();
    hr = m_commandList->Reset(m_allocators[m_currentBufferIndex].Get(), nullptr);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to reset command list. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }
  }

  void RenderContext::EndFrame() {
    HRESULT hr = S_OK;

    // TODO extract
    // Switch to present mode
    if (m_backBufferStates[m_currentBufferIndex] != D3D12_RESOURCE_STATE_PRESENT) {
      D3D12_RESOURCE_BARRIER barrier = {};
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition.pResource = m_backBuffers[m_currentBufferIndex].Get();
      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      m_commandList->ResourceBarrier(1, &barrier);

      m_backBufferStates[m_currentBufferIndex] = D3D12_RESOURCE_STATE_PRESENT;
    }

    // Close list
    m_commandList->Close();

    // Execute commands
    ID3D12CommandList* commandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(1, commandLists);

    // Present
    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to present swap chain. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }

    // Switch to the next frame. Update fence
    m_fenceCounter += 1;
    m_fenceValues[m_currentBufferIndex] = m_fenceCounter;
    hr = m_commandQueue->Signal(m_fence.Get(), m_fenceCounter);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to signal fence. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }

    m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();
  }

  void RenderContext::BindBackBufferRT() {
    if (m_backBufferStates[m_currentBufferIndex] != D3D12_RESOURCE_STATE_RENDER_TARGET) {
      // Switch mode
      D3D12_RESOURCE_BARRIER barrier = {};
      barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      barrier.Transition.pResource = m_backBuffers[m_currentBufferIndex].Get();
      barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
      barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
      barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      m_commandList->ResourceBarrier(1, &barrier);

      // Update state
      m_backBufferStates[m_currentBufferIndex] = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    // Bind RT
    m_commandList->OMSetRenderTargets(1, &m_rtvHeapHandles[m_currentBufferIndex], FALSE, nullptr);
  }

  void RenderContext::ClearRT(glm::vec4 color) {
    const float clearColor[] = { color.r, color.g, color.b, color.a };
    m_commandList->ClearRenderTargetView(m_rtvHeapHandles[m_currentBufferIndex], clearColor, 0, nullptr);
  }

  ID3D12CommandList* RenderContext::GetCommandList() const {
    return m_commandList.Get();
  }

  void RenderContext::WaitForAll() {
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
    swapChainDesc.SampleDesc.Quality = 0;

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

    // Back buffers
    D3D12_DESCRIPTOR_HEAP_DESC rtDesc = {};
    rtDesc.NumDescriptors = kBufferCount;
    rtDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

    hr = device->CreateDescriptorHeap(&rtDesc, IID_PPV_ARGS(&m_rtvHeap));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create back buffer descriptor heap. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    // Depth buffer
    D3D12_DESCRIPTOR_HEAP_DESC depthDesc = {};
    depthDesc.NumDescriptors = 1;
    depthDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    hr = device->CreateDescriptorHeap(&depthDesc, IID_PPV_ARGS(&m_depthHeap));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create depth buffer descriptor heap. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
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
      // Get buffer
      hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to get buffer. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        return false;
      }

      // Set state
      m_backBufferStates[i] = D3D12_RESOURCE_STATE_PRESENT;

      // Create view
      device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
      m_rtvHeapHandles[i] = rtvHandle;

      // Next descriptor
      rtvHandle.ptr += m_rtvDescriptorSize;
    }

    return true;
  }

  bool RenderContext::CreateDepthStencilBuffer(u32 width, u32 height) {
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    // Texture
    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = width;
    desc.Height = height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = DXGI_FORMAT_D32_FLOAT;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    hr = device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &desc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      &clearValue,
      IID_PPV_ARGS(m_depthBuffer.ReleaseAndGetAddressOf())
    );
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create depth buffer. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    // View
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthHeap->GetCPUDescriptorHandleForHeapStart();
    device->CreateDepthStencilView(m_depthBuffer.Get(), nullptr, dsvHandle);

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

  bool RenderContext::HandleResize() {
    if (!m_wasResized) {
      return true;
    }

    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    WaitForAll();

    // Back buffers
    for (auto& buffer : m_backBuffers) {
      buffer.Reset();
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    hr = m_swapChain->GetDesc(&swapChainDesc);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to get swap chain description. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    hr = m_swapChain->ResizeBuffers(kBufferCount, m_width, m_height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to resize back buffers. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    for (u32 i = 0; i < kBufferCount; ++i) {
      m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
      device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, m_rtvHeapHandles[i]);
    }

    m_currentBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

    // Depth buffer
    if (!CreateDepthStencilBuffer(m_width, m_height)) {
      return false;
    }

    m_wasResized = false;
    return true;
  }
}
