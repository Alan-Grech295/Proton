#include "ptpch.h"
#include "Proton/Bindable/PixelShader.h"

namespace Proton
{
	PixelShader::PixelShader(WindowsGraphics& gfx, const std::wstring& path)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		D3DReadFileToBlob(path.c_str(), &pBlob);
		GetDevice(gfx)->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);
	}

	void PixelShader::Bind(WindowsGraphics& gfx) noexcept
	{
		GetContext(gfx)->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	}
}