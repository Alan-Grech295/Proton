#pragma once
#include "Proton\Renderer\Texture.h"
#include <wrl.h>
#include <d3d11.h>
#include <WICTextureLoader.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class DirectXTexture2D : public Texture2D
	{
	public:
		DirectXTexture2D(std::string path, int slot = 0);
		virtual ~DirectXTexture2D() {}

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void Load(std::string path) override;
		virtual Color GetPixel(int x, int y) const override;

		virtual bool HasAlpha() const override;

		virtual std::string GetUID() const noexcept override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Resource> pTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		int m_Slot = 0;
		std::string m_Path;
		bool hasAlpha;
		int width, height;
		BYTE* data;
	};
}