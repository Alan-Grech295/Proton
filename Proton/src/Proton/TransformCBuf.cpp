#include "ptpch.h"
#include "Proton/TransformCBuf.h"

namespace Proton
{
	TransformCBuf::TransformCBuf(WindowsGraphics& gfx, const Drawable& parent)
		:
		parent(parent)
	{
		if (!pVcbuf)
		{
			pVcbuf = std::make_unique<VertexConstantBuffer<DirectX::XMMATRIX>>(gfx);
		}
	}

	void TransformCBuf::Bind(WindowsGraphics& gfx) noexcept
	{
		pVcbuf->Update(gfx,
			DirectX::XMMatrixTranspose(
				parent.GetTransformXM() * 
				gfx.GetCamera() *
				gfx.GetProjection()
			)
		);
		pVcbuf->Bind(gfx);
	}

	std::unique_ptr<VertexConstantBuffer<DirectX::XMMATRIX>> TransformCBuf::pVcbuf;
}