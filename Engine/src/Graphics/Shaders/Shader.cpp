#include "Shader.hpp"

namespace Flame {
  bool Shader::Compile(const std::wstring& filename, const ShaderDesc& desc) {
    return Compile(filename, desc.entryPoint, desc.shaderModel);
  }

  bool Shader::Compile(const std::wstring& filename, std::string_view entryPoint, std::string_view shaderModel) {
    HRESULT hr;
    ComPtr<ID3DBlob> errorBlob;
    u32 flags = D3DCOMPILE_ENABLE_STRICTNESS;

#ifdef _DEBUG
    flags |= D3DCOMPILE_DEBUG;
    flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    hr = D3DCompileFromFile(
      filename.c_str(),
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      entryPoint.data(),
      shaderModel.data(),
      flags,
      0,
      m_blob.ReleaseAndGetAddressOf(),
      errorBlob.GetAddressOf()
    );
    if (FAILED(hr)) {
      auto errorString = std::string(static_cast<const char*>(errorBlob->GetBufferPointer()));
      auto errorStringW = StringHelper::Utf8ToWide(errorString);
      Logger::Log(std::source_location::current(), LogLevel::Error, L"Failed to compile vertex shader: \"%s\"", errorStringW);
      return false;
    }

    return true;
  }

  void Shader::Reset() {
    m_blob.Reset();
  }

  ID3DBlob* Shader::GetBlob() const {
    return m_blob.Get();
  }

  D3D12_SHADER_BYTECODE Shader::GetBytecode() const {
    D3D12_SHADER_BYTECODE bytecode;
    bytecode.pShaderBytecode = m_blob->GetBufferPointer();
    bytecode.BytecodeLength = m_blob->GetBufferSize();
    return bytecode;
  }
}
