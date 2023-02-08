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
		//TODO: Pass by description
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

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

