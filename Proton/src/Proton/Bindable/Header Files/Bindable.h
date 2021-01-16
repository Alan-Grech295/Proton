#pragma once
#include "Platform/Windows/WindowsGraphics.h"

namespace Proton
{
	class Bindable
	{
	public:
		virtual void Bind(WindowsGraphics& gfx) noexcept = 0;
		virtual ~Bindable() = default;
	protected:
		static ID3D11DeviceContext* GetContext(WindowsGraphics& gfx) noexcept;
		static ID3D11Device* GetDevice(WindowsGraphics& gfx) noexcept;
	};
}