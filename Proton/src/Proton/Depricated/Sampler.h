#pragma once
#include "Bindable.h"

namespace Proton
{
	class Sampler : public Bindable
	{
	public:
		Sampler(WindowsGraphics& gfx);
		void Bind(WindowsGraphics& gfx) noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
	};
}