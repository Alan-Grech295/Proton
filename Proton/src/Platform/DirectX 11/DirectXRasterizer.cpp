#include "ptpch.h"
#include "DirectXRasterizer.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"

namespace Proton
{
	DirectXRasterizer::DirectXRasterizer(const std::string& tag, bool twoSided)
		:
		m_Uid(Rasterizer::GenerateUID(tag, twoSided))
	{
		m_TwoSided = twoSided;

		D3D11_RASTERIZER_DESC desc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
		desc.CullMode = twoSided ? D3D11_CULL_NONE : D3D11_CULL_BACK;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateRasterizerState(&desc, &pRasterizer);
	}

	void DirectXRasterizer::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->RSSetState(pRasterizer.Get());
	}

	std::string DirectXRasterizer::GetUID() const noexcept
	{
		return m_Uid;
	}
}
