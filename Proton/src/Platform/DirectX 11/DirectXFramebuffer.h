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
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
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

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual void Bind() override;
	private:
		FramebufferDescription m_Desc;
		D3D11_VIEWPORT vp;

		std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
		FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

		std::vector<ID3D11Texture2D*> m_ColorAttachmentTextures;
		std::vector<ID3D11RenderTargetView*> m_ColorAttachmentRenderTargets;
		std::vector<ID3D11ShaderResourceView*> m_ColorAttachmentSRVs;

		ID3D11DepthStencilView* m_DepthStencilView = nullptr;
		ID3D11DepthStencilState* m_DSState = nullptr;
	};
}