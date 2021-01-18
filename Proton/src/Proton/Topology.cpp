#include "ptpch.h"
#include "Proton/Topology.h"

namespace Proton
{
	Topology::Topology(WindowsGraphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
		:
		type(type)
	{}

	void Topology::Bind(WindowsGraphics& gfx) noexcept
	{
		GetContext(gfx)->IASetPrimitiveTopology(type);
	}
}