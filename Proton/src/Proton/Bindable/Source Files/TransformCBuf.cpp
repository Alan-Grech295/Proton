#include "ptpch.h"
#include "Proton/Bindable/Header Files/TransformCBuf.h"

namespace Proton
{
	TransformCBuf::TransformCBuf(WindowsGraphics& gfx, const Drawable& parent)
		:
		vcbuf(gfx),
		parent(parent)
	{}

	void TransformCBuf::Bind(WindowsGraphics& gfx) noexcept
	{
		vcbuf.Update(gfx,
			DirectX::XMMatrixTranspose(
				parent.GetTransformXM() * gfx.GetProjection()
			)
		);
		vcbuf.Bind(gfx);
	}
}