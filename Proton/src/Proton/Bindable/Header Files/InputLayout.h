#pragma once
#include "Bindable.h"

namespace Proton
{
	class InputLayout : public Bindable
	{
	public:
		InputLayout(WindowsGraphics& gfx,
			const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout,
			ID3DBlob* pVertexShaderBytecode);
		void Bind(WindowsGraphics& gfx) noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	};
}