#include "ptpch.h"
#include "DirectXTexture.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"

namespace Proton
{
	DirectXTexture2D::DirectXTexture2D(std::string path, int slot)
		:
		m_Slot(slot)
	{
		HRESULT hr = DirectX::CreateWICTextureFromFile(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice(), 
														std::wstring(path.begin(), path.end()).c_str(), 
														&pTexture, 
														&pTextureView);

		if (FAILED(hr))
		{
			PT_CORE_ERROR("[TEXTURE PATH] {0}", path);
			GET_ERROR(hr);
		}
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
		HRESULT hr = DirectX::CreateWICTextureFromFile(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice(),
														std::wstring(path.begin(), path.end()).c_str(),
														&pTexture,
														&pTextureView);

		if (FAILED(hr))
		{
			GET_ERROR(hr);
		}
	}

	Color DirectXTexture2D::GetPixel(int x, int y) const
	{
		return Color();
	}
}
