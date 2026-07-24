#include "RenderContext.hpp"

#include <source_location>

#include "RenderCore.hpp"
#include "Shaders/Shader.hpp"
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

    if (!CreateSwapChain(handle, width, height)) {
      return false;
    }

    if (!CreateCommandAllocators()) {
      return false;
    }

    if (!CreateCommandList()) {
      return false;
    }

    if (!CreateDescriptorHeaps()) {
      return false;
    }

    if (!CreateBackBuffers()) {
      return false;
    }

    if (!CreateDepthStencilBuffer(width, height)) {
      return false;
    }

    return true;
  }

  void RenderContext::Cleanup() {
    RenderCore::Get()->GetCommandQueue().Flush();

    m_commandList.Reset();

    for (auto& allocator : m_allocators) {
      allocator.Reset();
    }

    for (auto& buffer : m_backBuffers) {
      buffer.Reset();
    }

    m_rtvHeap.Reset();
    m_swapChain.Reset();
  }

  void RenderContext::Resize(u32 width, u32 height) {
    if (m_width == width && m_height == height) {
      return;
    }

    m_wasResized = true;
    m_width = width;
    m_height = height;
  }

  void RenderContext::BeginFrame() {
    HRESULT hr = S_OK;

    // Wait for previous frame to finish
    RenderCore::Get()->GetCommandQueue().WaitForFence(m_fenceValues[m_currentBufferIndex]);

    if (!HandleResize()) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to handle resize");
      return;
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
    RenderCore::Get()->GetCommandQueue().ExecuteCommandList(m_commandList.Get());

    // Present
    hr = m_swapChain->Present(1, 0);
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to present swap chain. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
    }

    // Switch frames
    m_fenceValues[m_currentBufferIndex] = RenderCore::Get()->GetCommandQueue().Signal();
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
    auto depthHandle = m_depthHeap->GetCPUDescriptorHandleForHeapStart();
    m_commandList->OMSetRenderTargets(1, &m_rtvHeapHandles[m_currentBufferIndex], FALSE, &depthHandle);
  }

  void RenderContext::ClearRT(glm::vec4 color) {
    const float clearColor[] = { color.r, color.g, color.b, color.a };
    m_commandList->ClearRenderTargetView(m_rtvHeapHandles[m_currentBufferIndex], clearColor, 0, nullptr);
  }

  void RenderContext::ClearDepthStencil() {
    m_commandList->ClearDepthStencilView(m_depthHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
  }

  ID3D12CommandList* RenderContext::GetCommandList() const {
    return m_commandList.Get();
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
    hr = factory->CreateSwapChainForHwnd(RenderCore::Get()->GetCommandQueue().GetCommandQueue(), handle, &swapChainDesc, nullptr, nullptr, &swapChain);
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

    auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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
      rtvHandle.ptr += rtvDescriptorSize;
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

  bool RenderContext::ThisShit() {
    HRESULT hr = S_OK;
    auto* device = RenderCore::Get()->GetDevice();

    // Depth Stencil State
    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;
    depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
    depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

    depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    depthDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

    // Rasterizer State
    D3D12_RASTERIZER_DESC rasterizerDesc = {};
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;
    rasterizerDesc.ForcedSampleCount = 0;
    rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Render Target Blend State
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = FALSE;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlend = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    rtBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    // Blend State
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0] = rtBlendDesc;

    // Load Shaders
    Shader vertexShader;
    Shader pixelShader;
    if (!vertexShader.Compile(L"Assets/Shaders/Base.hlsl", Shader::VS_5_0)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to compile vertex shader");
      return false;
    }

    if (!pixelShader.Compile(L"Assets/Shaders/Base.hlsl", Shader::PS_5_0)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to compile pixel shader");
      return false;
    }

    // Root Signature
    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.NumParameters = 0;
    rootDesc.pParameters = nullptr;
    rootDesc.NumStaticSamplers = 0;
    rootDesc.pStaticSamplers = nullptr;
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> errorBlob;
    hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.ReleaseAndGetAddressOf(), errorBlob.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to serialize root signature. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    ComPtr<ID3D12RootSignature> rootSignature;
    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(rootSignature.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create root signature. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
      return false;
    }

    // Input Layout
    D3D12_INPUT_ELEMENT_DESC inputDesc[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
    inputLayoutDesc.pInputElementDescs = inputDesc;
    inputLayoutDesc.NumElements = std::size(inputDesc);


    // Pipeline State Object
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc = {};
    stateDesc.pRootSignature = rootSignature.Get();
    stateDesc.VS = vertexShader.GetBytecode();
    stateDesc.PS = pixelShader.GetBytecode();
    stateDesc.StreamOutput = {};
    stateDesc.BlendState = blendDesc;
    stateDesc.SampleMask = UINT_MAX;
    stateDesc.RasterizerState = rasterizerDesc;
    stateDesc.DepthStencilState = depthDesc;
    stateDesc.InputLayout = inputLayoutDesc;
    stateDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    stateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    stateDesc.NumRenderTargets = 1;
    stateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    stateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    stateDesc.SampleDesc.Count = 1;
    stateDesc.SampleDesc.Quality = 0;
    stateDesc.NodeMask = 0;
    stateDesc.CachedPSO = {};
    stateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ComPtr<ID3D12PipelineState> pipelineState;
    hr = device->CreateGraphicsPipelineState(&stateDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    if (FAILED(hr)) {
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to create pipeline state. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
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

    RenderCore::Get()->GetCommandQueue().Flush();

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
      hr = m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_backBuffers[i]));
      if (FAILED(hr)) {
        Logger::Log(std::source_location::current(), LogLevel::Warning, L"Failed to get the back buffer. Code: {:#X} ({})", (u64)hr, LogHelper::GetHresultString(hr));
        return false;
      }

      device->CreateRenderTargetView(m_backBuffers[i].Get(), nullptr, m_rtvHeapHandles[i]);
      m_backBufferStates[i] = D3D12_RESOURCE_STATE_PRESENT;
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
