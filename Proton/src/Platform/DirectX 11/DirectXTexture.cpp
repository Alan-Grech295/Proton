#include "ptpch.h"
#include "DirectXTexture.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"
#include <cassert>
#include "Proton\Asset Loader\AssetManager.h"

namespace Proton
{
	DirectXTexture2D::DirectXTexture2D(std::string path, int slot)
	{
		m_Slot = slot;
		m_Path = path;
		Load(path);
	}

	void DirectXTexture2D::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetShaderResources(m_Slot, 1, pTextureView.GetAddressOf());
	}

	void DirectXTexture2D::Load(std::string path)
	{
		Ref<Image> image = AssetManager::GetImage(path);

		if (image == nullptr)
		{
			image = CreateRef<Image>();
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

			image->width = (uint32_t)scratch.GetMetadata().width;
			image->height = (uint32_t)scratch.GetMetadata().height;
			image->isOpaque = scratch.IsAlphaAllOpaque();
			image->pixels = new uint8_t[scratch.GetPixelsSize()];
			image->pixels = scratch.GetPixels();
		}

		//TODO: Pass texture description through constructor
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Format = format;
		desc.Width = image->width;
		desc.Height = image->height;
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		/*D3D11_SUBRESOURCE_DATA srd = {};
		srd.pSysMem = image->pixels;
		srd.SysMemPitch = image->width * 4 * sizeof(uint8_t);*/

		/*for (int i = 0; i < image->width * image->height * 4; i++)
			uint8_t pix = image->pixels[i];*/

		HRESULT hr = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&desc, nullptr, &pTexture);

		if (FAILED(hr))
		{
			LAST_ERROR(hr);
		}

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->UpdateSubresource(
			pTexture.Get(), 0, nullptr, image->pixels, image->width * 4, 0
		);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateShaderResourceView(pTexture.Get(), &srvDesc, &pTextureView);

		hasAlpha = !image->isOpaque;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->GenerateMips(pTextureView.Get());
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

	void* DirectXTexture2D::GetTexturePointer() const
	{
		return (void*)pTextureView.Get();
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
