#pragma once
#include "ConstantBuffer.h"
#include "Proton/Drawable/Drawable.h"
#include <DirectXMath.h>

namespace Proton
{
	class TransformCBuf : public Bindable
	{
	public:
		TransformCBuf(WindowsGraphics& gfx, const Drawable& parent);
		void Bind(WindowsGraphics& gfx) noexcept override;
	private:
		static std::unique_ptr<VertexConstantBuffer<DirectX::XMMATRIX>> pVcbuf;
		const Drawable& parent;
	};
}