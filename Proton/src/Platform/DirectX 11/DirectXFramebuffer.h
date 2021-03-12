#pragma once
#include "Proton\Renderer\Framebuffer.h"
#include <wrl.h>
#include <d3d11.h>
#include <Windows.h>

#pragma comment(lib, "d3d11.lib")


namespace Proton
{
	class DirectXFramebuffer : public Framebuffer
	{
	public:
		DirectXFramebuffer(const FramebufferDescription& desc);
		virtual ~DirectXFramebuffer();
		void Recreate();

		virtual const FramebufferDescription& GetDescription() const override { return m_Desc; }

		virtual void* GetRenderTextureID() override;

		virtual void Clear() override;
		virtual void SetClearCol(float r, float g, float b) override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind() override;
		virtual void Unbind() override;
	private:
		FramebufferDescription m_Desc;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
		D3D11_VIEWPORT vp;
		float* clearCol;
	};
}