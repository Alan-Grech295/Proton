#include "ptpch.h"
#include "DirectXBlender.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"

namespace Proton
{
	DirectXBlender::DirectXBlender(const std::string& tag, bool blending)
		:
		m_Uid(Blender::GenerateUID(tag, blending))
	{
		m_Blending = blending;

		D3D11_BLEND_DESC desc = {};
		auto& brt = desc.RenderTarget[0];

		if (blending)
		{
			brt.BlendEnable = TRUE;
			brt.SrcBlend = D3D11_BLEND_SRC_ALPHA;
			brt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			brt.BlendOp = D3D11_BLEND_OP_ADD;
			brt.SrcBlendAlpha = D3D11_BLEND_ZERO;
			brt.DestBlendAlpha = D3D11_BLEND_ZERO;
			brt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
			brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}
		else
		{
			brt.BlendEnable = FALSE;
			brt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		}

		DX_CHECK_ERROR(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBlendState(&desc, &pBlendState));
	}

	void DirectXBlender::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetBlendState(pBlendState.Get(), nullptr, 0xFFFFFFFF);
	}

	std::string DirectXBlender::GetUID() const noexcept
	{
		return m_Uid;
	}
}

