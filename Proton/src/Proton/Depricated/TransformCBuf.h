#pragma once
#include "ConstantBuffer.h"
#include "Proton/Drawable.h"
#include <DirectXMath.h>

namespace Proton
{
	class TransformCBuf : public Bindable
	{
	public:
		TransformCBuf(WindowsGraphics& gfx, const Drawable& parent, UINT slot = 0);
		void Bind(WindowsGraphics& gfx) noexcept override;
	private:
		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	private:
		static std::unique_ptr<VertexConstantBuffer<TransformCBuf::Transforms>> pVcbuf;
		const Drawable& parent;
	};
}