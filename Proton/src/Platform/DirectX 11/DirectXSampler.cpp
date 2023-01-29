#include "ptpch.h"
#include "DirectXSampler.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"

namespace Proton
{
	DirectXSampler::DirectXSampler(const std::string& tag, int slot)
		:
		uid(tag)
	{
		m_Slot = slot;

		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateSamplerState(&samplerDesc, &pSampler);
	}

	void DirectXSampler::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetSamplers(m_Slot, 1, pSampler.GetAddressOf());
	}

	std::string DirectXSampler::GetUID() const noexcept
	{
		return uid;
	}
}

