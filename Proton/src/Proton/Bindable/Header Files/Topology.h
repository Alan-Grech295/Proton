#pragma once
#include "Bindable.h"

namespace Proton
{
	class Topology : public Bindable
	{
	public:
		Topology(WindowsGraphics&gfx, D3D11_PRIMITIVE_TOPOLOGY type);
		void Bind(WindowsGraphics& gfx) noexcept override;
	protected:
		D3D11_PRIMITIVE_TOPOLOGY type;
	};
}