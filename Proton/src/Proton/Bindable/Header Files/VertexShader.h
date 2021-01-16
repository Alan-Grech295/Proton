#pragma once
#include "Bindable.h"

namespace Proton
{
	class VertexShader : public Bindable
	{
	public:
		VertexShader(WindowsGraphics& gfx, const std::wstring& path);
		void Bind(WindowsGraphics& gfx) noexcept override;
		ID3DBlob* GetBytecode() const noexcept;
	protected:
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	};
}