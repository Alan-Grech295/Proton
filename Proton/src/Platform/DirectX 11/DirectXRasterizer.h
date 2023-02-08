#pragma once
#include "Proton\Renderer\Bindables\Rasterizer.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class DirectXRasterizer : public Rasterizer
	{
	public:
		DirectXRasterizer(const std::string& tag, bool twoSided);
		virtual ~DirectXRasterizer() {}

		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
		std::string m_Uid;
	};
}