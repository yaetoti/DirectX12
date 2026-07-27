#include "CommandQueue.hpp"

#include "RenderCore.hpp"
#include "Utils/Logger.hpp"

namespace Flame {
  CommandQueue::~CommandQueue() {
    Reset();
  }

  bool CommandQueue::Initialize() {
    if (!CreateCommandQueue()) {
      return false;
    }

    if (!CreateFence()) {
      return false;
    }

    return true;
  }

  void CommandQueue::Reset() {
    if (m_commandQueue) {
      Flush();
    }

    if (m_fenceEvent) {
      CloseHandle(m_fenceEvent);
      m_fenceEvent = nullptr;
    }

    m_fence.Reset();
    m_commandQueue.Reset();
  }

  void CommandQueue::Execute(ID3D12CommandList* commandList) {
    m_commandQueue->ExecuteCommandLists(1, &commandList);
  }

  u64 CommandQueue::Signal() {
    u64 fenceValue = m_nextFenceValue;
    HRESULT hr = m_commandQueue->Signal(m_fence.Get(), fenceValue);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to signal fence. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      throw std::runtime_error("Failed to signal fence");
    }

    m_nextFenceValue += 1;
    return fenceValue;
  }

  void CommandQueue::WaitForFence(u64 fenceValue) {
    if (m_fence->GetCompletedValue() < fenceValue) {
      HRESULT hr = m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to set fence event. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        return;
      }

      DWORD result = WaitForSingleObject(m_fenceEvent, INFINITE);
      if (result != WAIT_OBJECT_0) {
        Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to wait for fence event");
        return;
      }
    }
  }

  void CommandQueue::Flush() {
    u64 value = Signal();
    WaitForFence(value);
  }

  ID3D12CommandQueue* CommandQueue::GetCommandQueue() const {
    return m_commandQueue.Get();
  }

  bool CommandQueue::CreateCommandQueue() {
    HRESULT hr;
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

  bool CommandQueue::CreateFence() {
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
}
