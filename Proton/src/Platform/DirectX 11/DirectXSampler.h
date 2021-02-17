#pragma once
#include "Proton\Renderer\Sampler.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class DirectXSampler : public Sampler
	{
	public:
		DirectXSampler(int slot = 0);
		virtual ~DirectXSampler() {}

		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		int m_Slot = 0;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
	};
}