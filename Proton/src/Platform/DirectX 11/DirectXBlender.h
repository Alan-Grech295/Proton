#pragma once
#include "Proton\Renderer\Bindables\Blender.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class DirectXBlender : public Blender
	{
	public:
		DirectXBlender(const std::string& tag, bool blending);
		virtual ~DirectXBlender() {}

		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		Microsoft::WRL::ComPtr<ID3D11BlendState> pBlendState;
		std::string m_Uid;
	};
}