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
			pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx);
		}
	}

	void TransformCBuf::Bind(WindowsGraphics& gfx) noexcept
	{
		const auto model = parent.GetTransformXM();
		const Transforms tf =
		{
			DirectX::XMMatrixTranspose(model),
			DirectX::XMMatrixTranspose(
				model *
				gfx.GetCamera() *
				gfx.GetProjection())
		};
		pVcbuf->Update(gfx, tf);
		pVcbuf->Bind(gfx);
	}

	std::unique_ptr<VertexConstantBuffer<TransformCBuf::Transforms>> TransformCBuf::pVcbuf;
}