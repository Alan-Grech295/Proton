#pragma once
#include "Proton\Renderer\Bindables\Sampler.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class DirectXSampler : public Sampler
	{
	public:
		DirectXSampler(const std::string& tag, int slot = 0);
		virtual ~DirectXSampler() {}

		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		int m_Slot = 0;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		std::string uid;
	};
}