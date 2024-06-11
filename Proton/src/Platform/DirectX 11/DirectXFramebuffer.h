#pragma once
#include "Proton\Renderer\Framebuffer.h"
#include <wrl.h>
#include <d3d11.h>
#include <Windows.h>

#pragma comment(lib, "d3d11.lib")


namespace Proton
{
	struct DepthAttachment
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthTexture;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;

		operator bool() 
		{
			return pDepthStencilView;
		}

		void Release()
		{
			pDepthStencilView->Release();
			pDSState->Release();
			pDepthTexture->Release();
		}
	};

	struct ColorAttachment
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pRenderTarget;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV;

		operator bool()
		{
			return pRenderTarget;
		}

		void Release()
		{
			pTexture->Release();
			pRenderTarget->Release();
			pSRV->Release();
		}
	};

	class DirectXFramebuffer : public Framebuffer
	{
	public:
		DirectXFramebuffer(const FramebufferDescription& desc);
		virtual ~DirectXFramebuffer();
		void Recreate();

		virtual const FramebufferDescription& GetDescription() const override { return m_Desc; }

		virtual void* GetRenderTextureID(uint32_t index) override;

		virtual void Clear() override;
		virtual void ClearDepth() override;

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void ReadPixel_Impl(uint32_t targetIndex, bool depth, int x, int y, uint32_t size, void* dest) override;

		virtual void Bind() override;
	private:
		void Invalidate();
		void ClearRenderTarget(ID3D11RenderTargetView* target, FramebufferTextureSpecification& spec);
	private:
		FramebufferDescription m_Desc;
		D3D11_VIEWPORT vp;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = { FramebufferTextureFormat::None, 0 };

		std::vector<ColorAttachment> m_ColorAttachments;
		std::vector<ID3D11RenderTargetView*> m_RenderTargets;

		DepthAttachment m_DepthAttachment = {};
	};
}