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
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();
    auto* factory = RenderCore::Get()->GetFactory();

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

    // Swap Chain
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

    // Descriptor Heap
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
    heapDesc.NumDescriptors = kBufferCount;
    heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_rtvHeap));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create descriptor heap. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    // Buffers
    u64 rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
    for (u32 i = 0; i < kBufferCount; ++i) {
      hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to get buffer. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        return false;
      }

      device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, rtvHandle);
      rtvHandle.ptr += rtvDescriptorSize;
    }

    // Fence
    hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create fence. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    m_fenceEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);

    return true;
  }

  void RenderContext::Cleanup() {
    if (m_fenceEvent) {
      CloseHandle(m_fenceEvent);
      m_fenceEvent = nullptr;
    }

    for (auto& buffer : m_backBuffers) {
      buffer.Reset();
    }

    m_fence.Reset();
    m_rtvHeap.Reset();
    m_swapChain.Reset();
    m_commandQueue.Reset();
  }
}
