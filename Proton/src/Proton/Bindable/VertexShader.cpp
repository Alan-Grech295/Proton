#include "ptpch.h"
#include "Proton/Bindable/VertexShader.h"

namespace Proton
{
	VertexShader::VertexShader(WindowsGraphics& gfx, const std::wstring& path)
	{
		D3DReadFileToBlob(path.c_str(), &pBytecodeBlob);
		GetDevice(gfx)->CreateVertexShader(
			pBytecodeBlob->GetBufferPointer(),
			pBytecodeBlob->GetBufferSize(),
			nullptr,
			&pVertexShader
		);
	}

	void VertexShader::Bind(WindowsGraphics& gfx) noexcept
	{
		GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	}

	ID3DBlob* VertexShader::GetBytecode() const noexcept
	{
		return pBytecodeBlob.Get();
	}
}