#pragma once
#include "DrawableBase.h"

namespace Proton
{
	class SolidSphere : public DrawableBase<SolidSphere>
	{
	public:
		SolidSphere(WindowsGraphics& gfx, float radius);
		void Update(float dt) noexcept override;
		void SetPos(DirectX::XMFLOAT3 pos);
		DirectX::XMMATRIX GetTransformXM() const noexcept override;
	private:
		DirectX::XMFLOAT3 pos = { 1.0f, 1.0f, 1.0f };
	};
}