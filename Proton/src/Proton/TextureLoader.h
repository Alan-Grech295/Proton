#pragma once
#include <Windows.h>
#include <string>
#include <assert.h>
#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

namespace Proton
{
	class TextureLoader
	{
	public:
		static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureView(ID3D11Device* device, const std::string& path);
		static Microsoft::WRL::ComPtr<ID3D11Resource> GetTexture(ID3D11Device* device, const std::string& path);
	};
}