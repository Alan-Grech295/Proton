#include "ptpch.h"
#include "Proton/Bindable/Bindable.h"

namespace Proton
{
	ID3D11DeviceContext* Bindable::GetContext(WindowsGraphics& gfx) noexcept
	{
		return gfx.pContext.Get();
	}

	ID3D11Device* Bindable::GetDevice(WindowsGraphics& gfx) noexcept
	{
		return gfx.pDevice.Get();
	}
}