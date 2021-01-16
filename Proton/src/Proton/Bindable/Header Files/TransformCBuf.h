#pragma once
#include "ConstantBuffer.h"
#include "Proton/Drawable/Header Files/Drawable.h"
#include <DirectXMath.h>

namespace Proton
{
	class TransformCBuf : public Bindable
	{
	public:
		TransformCBuf(WindowsGraphics& gfx, const Drawable& parent);
		void Bind(WindowsGraphics& gfx) noexcept override;
	private:
		VertexConstantBuffer<DirectX::XMMATRIX> vcbuf;
		const Drawable& parent;
	};
}