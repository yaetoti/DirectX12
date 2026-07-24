#include "RenderCore.hpp"

#include <cassert>

#include "Utils/Logger.hpp"
#include "Utils/Types.hpp"

namespace Flame {
  RenderCore::RenderCore() {
  }

  RenderCore::~RenderCore() {
    Cleanup();
  }

  void RenderCore::Start() {
    assert(s_instance == nullptr);
    s_instance = new RenderCore();
  }

  void RenderCore::Shutdown() {
    assert(s_instance != nullptr);
    delete s_instance;
  }

  RenderCore* RenderCore::Get() {
    assert(s_instance != nullptr);
    return s_instance;
  }

  bool RenderCore::Initialize() {
#ifdef _DEBUG
    if (!CreateDebugLayer()) {
      return false;
    }
#endif

    if (!CreateFactory()) {
      return false;
    }

    if (!SelectHardwareAdapter()) {
      return false;
    }

    if (!CreateDevice()) {
      return false;
    }

    if (!m_commandQueue.Initialize()) {
      return false;
    }

    return true;
  }

  void RenderCore::Cleanup() {
    m_commandQueue.Reset();
    m_device.Reset();
    m_adapter.Reset();
    m_factory.Reset();
    m_debug.Reset();
  }

  ID3D12Debug1* RenderCore::GetDebug() {
    return m_debug.Get();
  }

  IDXGIFactory7* RenderCore::GetFactory() {
    return m_factory.Get();
  }

  IDXGIAdapter4* RenderCore::GetAdapter() {
    return m_adapter.Get();
  }

  ID3D12Device9* RenderCore::GetDevice() {
    return m_device.Get();
  }

  CommandQueue& RenderCore::GetCommandQueue() {
    return m_commandQueue;
  }

  bool RenderCore::CreateDebugLayer() {
    HRESULT hr = S_OK;

    hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_debug));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to get debug interface. Code: 0x{:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    m_debug->EnableDebugLayer();
    m_debug->SetEnableGPUBasedValidation(true);
    return true;
  }

  bool RenderCore::CreateFactory() {
    HRESULT hr = S_OK;
    u32 flags = 0;

#ifdef _DEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    hr = CreateDXGIFactory2(flags, IID_PPV_ARGS(&m_factory));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create DXGI factory. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    return true;
  }

  bool RenderCore::SelectHardwareAdapter() {
    HRESULT hr = S_OK;
    ComPtr<IDXGIAdapter1> adapter;
    u32 highestVideoMemory = 0;

    for (u32 i = 0;; ++i) {
      if (FAILED(m_factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)))) {
        break;
      }

      DXGI_ADAPTER_DESC1 desc;
      hr = adapter->GetDesc1(&desc);
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to get adapter description. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        continue;
      }

      if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
        continue;
      }

      if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr))) {
        continue;
      }

      if (desc.DedicatedVideoMemory > highestVideoMemory) {
        highestVideoMemory = desc.DedicatedVideoMemory;
        adapter.As(&m_adapter);
      }
    }

    if (m_adapter == nullptr) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to find a suitable hardware adapter");
      return false;
    }

    return true;
  }

  bool RenderCore::CreateDevice() {
    if (FAILED(D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)))) {
      return false;
    }

#ifdef _DEBUG
    ComPtr<ID3D12InfoQueue> infoQueue;
    if (SUCCEEDED(m_device.As(&infoQueue))) {
      infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
      infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
      infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
    }
    else {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to get info queue");
    }
#endif

    return true;
  }
}
