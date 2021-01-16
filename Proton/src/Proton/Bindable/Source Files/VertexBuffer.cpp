#include "ptpch.h"
#include "Proton/Bindable/Header Files/VertexBuffer.h"

namespace Proton
{
	void VertexBuffer::Bind(WindowsGraphics& gfx) noexcept
	{
		const UINT offset = 0u;
		GetContext(gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
	}
}