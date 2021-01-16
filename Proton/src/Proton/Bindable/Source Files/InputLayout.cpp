#include "ptpch.h"
#include "Proton/Bindable/Header Files/InputLayout.h"

namespace Proton
{
	InputLayout::InputLayout(WindowsGraphics& gfx,
		const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
		ID3DBlob* pVertexShaderBytecode)
	{
		GetDevice(gfx)->CreateInputLayout(
			layout.data(), (UINT)layout.size(),
			pVertexShaderBytecode->GetBufferPointer(),
			pVertexShaderBytecode->GetBufferSize(),
			&pInputLayout
		);
	}

	void InputLayout::Bind(WindowsGraphics& gfx) noexcept
	{
		GetContext(gfx)->IASetInputLayout(pInputLayout.Get());
	}
}
