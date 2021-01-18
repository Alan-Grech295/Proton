#pragma once
#include "Bindable.h"

namespace Proton
{
	class Texture : public Bindable
	{
	public:
		//Texture(WindowsGraphics& gfx, const class Surface& s);
		Texture(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView);
		void Bind(WindowsGraphics& gfx) noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
	};
}