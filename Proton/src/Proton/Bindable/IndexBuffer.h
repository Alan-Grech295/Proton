#pragma once
#include "Bindable.h"

namespace Proton
{
	class IndexBuffer : public Bindable
	{
	public:
		IndexBuffer(WindowsGraphics& gfx, const std::vector<unsigned short>& indices);
		void Bind(WindowsGraphics& gfx) noexcept override;
		UINT GetCount() const noexcept;
	protected:
		UINT count;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	};
}