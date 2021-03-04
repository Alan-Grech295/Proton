#include "ptpch.h"
#include "DirectXTexture.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"

namespace Proton
{
	DirectXTexture2D::DirectXTexture2D(std::string path, int slot)
		:
		m_Slot(slot),
		m_Path(path)
	{
		Load(path);
	}

	void DirectXTexture2D::Bind() const
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetShaderResources(m_Slot, 1, pTextureView.GetAddressOf());
	}

	void DirectXTexture2D::Unbind() const
	{

	}

	void DirectXTexture2D::Load(std::string path)
	{
		HRESULT hr = DirectX::CreateWICTextureFromFileEx(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice(),
			std::wstring(path.begin(), path.end()).c_str(), 0, D3D11_USAGE_DEFAULT,
			D3D11_BIND_SHADER_RESOURCE, 0, 0, DirectX::WIC_LOADER_DEFAULT,
			&pTexture,
			&pTextureView);


		ID3D11Texture2D* tex = static_cast<ID3D11Texture2D*>(pTexture.Get());

		D3D11_TEXTURE2D_DESC desc;

		tex->GetDesc(&desc);
		width = desc.Width;
		height = desc.Height;

		ID3D11Texture2D* transferTex;

		D3D11_TEXTURE2D_DESC desc2;
		desc2 = desc;
		desc2.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc2.Usage = D3D11_USAGE_STAGING;
		desc2.BindFlags = 0;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&desc2, nullptr, &transferTex);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->CopyResource(transferTex, tex);

		D3D11_MAPPED_SUBRESOURCE msr;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
			transferTex, 0,
			D3D11_MAP_READ, 0,
			&msr
		);

		data = static_cast<BYTE*>(msr.pData);

		hasAlpha = false;

		for (int i = 0; i < desc.Height * desc.Width * 4; i += 4)
		{
			if (data[i + 3] != 255)
			{
				hasAlpha = true;
				break;
			}
		}

		if (FAILED(hr))
		{
			PT_CORE_ERROR("[TEXTURE PATH] {0}", path);
			GET_ERROR(hr);
		}
	}

	Color DirectXTexture2D::GetPixel(int x, int y) const
	{
		int arrStart = (x + y * width) * 4;
		Color c{ data[arrStart] / 255.0f, data[arrStart + 1] / 255.0f, 
				 data[arrStart + 2] / 255.0f, data[arrStart + 3] / 255.0f };
		return c;
	}

	bool DirectXTexture2D::HasAlpha() const
	{
		return hasAlpha;
	}

	std::string DirectXTexture2D::GetUID() const noexcept
	{
		return m_Path;
	}
}
