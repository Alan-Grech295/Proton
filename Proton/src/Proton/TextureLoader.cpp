#include "ptpch.h"
#include "TextureLoader.h"
#include "Log.h"

#pragma comment( lib,"d3d11.lib" )
#pragma comment( lib,"DirectXTK.lib" )

namespace Proton
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureLoader::GetTextureView(ID3D11Device* device, const std::string& path)
	{
		// convert filenam to wide string
		wchar_t wideName[512];
		mbstowcs_s(nullptr, wideName, path.c_str(), _TRUNCATE);

		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;

		HRESULT hr = DirectX::CreateWICTextureFromFile(device, wideName, nullptr, &textureSRV);

		if (FAILED(hr))
		{
			GET_ERROR(hr);
		}

		return textureSRV;
	}

	Microsoft::WRL::ComPtr<ID3D11Resource> TextureLoader::GetTexture(ID3D11Device* device, const std::string& path)
	{
		// convert filenam to wide string
		wchar_t wideName[512];
		mbstowcs_s(nullptr, wideName, path.c_str(), _TRUNCATE);

		Microsoft::WRL::ComPtr<ID3D11Resource> texture;

		HRESULT hr = DirectX::CreateWICTextureFromFile(device, wideName, &texture, nullptr);

		if (FAILED(hr))
		{
			GET_ERROR(hr);
		}

		return texture;
	}
}


