#include "ptpch.h"
#include "DirectXTexture.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"
#include <cassert>

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
		HRESULT hr = DirectX::LoadFromWICFile(std::wstring(path.begin(), path.end()).c_str(), DirectX::WIC_FLAGS_NONE,
											  nullptr, scratch);

		if (FAILED(hr))
		{
			PT_CORE_ERROR("[TEXTURE PATH] {0}", path);
			GET_ERROR(hr);
		}

		if (scratch.GetImage(0, 0, 0)->format != format)
		{
			DirectX::ScratchImage converted;
			hr = DirectX::Convert(
				*scratch.GetImage(0, 0, 0),
				format,
				DirectX::TEX_FILTER_DEFAULT,
				DirectX::TEX_THRESHOLD_DEFAULT,
				converted
			);

			scratch = std::move(converted);
		}

		width = scratch.GetMetadata().width;
		height = scratch.GetMetadata().height;

		D3D11_TEXTURE2D_DESC desc = {};
		desc.Format = format;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA srd;
		srd.pSysMem = scratch.GetPixels();
		srd.SysMemPitch = scratch.GetImage(0, 0, 0)->rowPitch;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&desc, &srd, &pTexture);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTextureView);

		hasAlpha = !scratch.IsAlphaAllOpaque();
	}

	Color DirectXTexture2D::GetPixel(int x, int y) const
	{
		assert(x >= 0);
		assert(y >= 0);
		//assert(x < GetWidth());
		//assert(y < GetHeight());
		//auto& imgData = *scratch.GetImage(0, 0, 0);

		return Color();// reinterpret_cast<Color*>(&imgData.pixels[y * imgData.rowPitch])[x];
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
