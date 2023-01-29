#pragma once
#include "Proton\Renderer\Bindables\Texture.h"
#include <wrl.h>
#include <DirectXTex.h>

namespace Proton
{
	class DirectXTexture2D : public Texture2D
	{
	public:
		DirectXTexture2D(std::string path, int slot = 0);
		//DirectXTexture2D(uint32_t assetID, int slot = 0);
		virtual ~DirectXTexture2D() {}

		virtual void Bind() override;

		virtual void Load(std::string path) override;
		//virtual void Load(uint32_t assetID) override;
		virtual Color GetPixel(int x, int y) const override;
		virtual void* GetTexturePointer() const override;

		virtual bool HasAlpha() const override;

		virtual std::string GetUID() const noexcept override;
	public:
		static constexpr DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		bool hasAlpha;
		int width, height;
		DirectX::ScratchImage scratch;

	};
}











