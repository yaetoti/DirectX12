#pragma once
#include <d3d12.h>
#include <d3dcompiler.h>
#include <string>

#include "Utils/Logger.hpp"
#include "Utils/WinTypes.hpp"

namespace Flame {
  struct ShaderDesc final {
    std::string_view entryPoint;
    std::string_view shaderModel;
  };

  struct Shader {
    static constexpr ShaderDesc VS_5_0 = { "VSMain", "vs_5_0" };
    static constexpr ShaderDesc HS_5_0 = { "HSMain", "hs_5_0" };
    static constexpr ShaderDesc DS_5_0 = { "DSMain", "ds_5_0" };
    static constexpr ShaderDesc GS_5_0 = { "GSMain", "gs_5_0" };
    static constexpr ShaderDesc PS_5_0 = { "PSMain", "ps_5_0" };

    bool Compile(const std::wstring& filename, const ShaderDesc& desc);
    bool Compile(const std::wstring& filename, std::string_view entryPoint, std::string_view shaderModel);
    void Reset();

    ID3DBlob* GetBlob() const;
    D3D12_SHADER_BYTECODE GetBytecode() const;

  private:
    ComPtr<ID3DBlob> m_blob;
  };
}
