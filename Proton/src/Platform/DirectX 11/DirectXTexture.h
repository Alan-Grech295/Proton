#pragma once
#include "Proton\Renderer\Texture.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class DirectXTexture2D : public Texture2D
	{
	public:
		DirectXTexture2D(std::string path);
		virtual ~DirectXTexture2D() {}

		virtual void Bind(int slot = 0) const override;
		virtual void Unbind() const override;
		virtual void Load(std::string path) override;
		virtual Color GetPixel(int x, int y) const override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Resource> pTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
	};
}