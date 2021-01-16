#pragma once
#include "Bindable.h"

namespace Proton
{
	class PixelShader : public Bindable
	{
	public:
		PixelShader(WindowsGraphics& gfx, const std::wstring& path);
		void Bind(WindowsGraphics& gfx) noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	};
}