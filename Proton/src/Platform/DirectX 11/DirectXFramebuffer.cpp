#include "ptpch.h"
#include "DirectXFramebuffer.h"
#include "DirectXRendererAPI.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Debugging/Exceptions.h"

namespace Proton
{
	namespace Utils {
		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH32F: return true;
			}

			return false;
		}

		static void CreateRenderTarget(ID3D11Texture2D** texture, ID3D11RenderTargetView** renderTarget, ID3D11ShaderResourceView** srv, DXGI_FORMAT format, uint32_t samples, uint32_t width, uint32_t height)
		{
			D3D11_TEXTURE2D_DESC textureDesc = {};
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};

			// Setup the render target texture description.
			textureDesc.Width = width;
			textureDesc.Height = height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			//TODO: Check texture spec format
			textureDesc.Format = format;
			textureDesc.SampleDesc.Count = samples;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;

			// Create the render target texture.
			GFX_THROW_INFO(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&textureDesc, NULL, texture));

			// Setup the description of the render target view.
			renderTargetViewDesc.Format = format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;

			// Create the render target view.
			GFX_THROW_INFO(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateRenderTargetView(*texture, &renderTargetViewDesc, renderTarget));

			// Setup the description of the shader resource view.
			shaderResourceViewDesc.Format = format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;

			// Create the shader resource view.
			GFX_THROW_INFO(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateShaderResourceView(*texture, &shaderResourceViewDesc, srv));
		}
	
		static void CreateDepthTexture(ID3D11DepthStencilView** depthStencilView, ID3D11DepthStencilState** depthStencilState, DXGI_FORMAT format, uint32_t width, uint32_t height)
		{
			//Create depth stencil state
			D3D11_DEPTH_STENCIL_DESC dsDesc = {};
			dsDesc.DepthEnable = TRUE;
			dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

			GFX_THROW_INFO(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateDepthStencilState(&dsDesc, depthStencilState));

			//Create depth stencil texture
			Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = width;
			descDepth.Height = height;
			descDepth.MipLevels = 1;
			descDepth.ArraySize = 1;
			descDepth.Format = format;// DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1;
			descDepth.SampleDesc.Quality = 0;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			descDepth.CPUAccessFlags = 0;

			GFX_THROW_INFO(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

			//Create depth stencil view
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0;

			GFX_THROW_INFO(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, depthStencilView));
		}
	}

	DirectXFramebuffer::DirectXFramebuffer(const FramebufferDescription& desc)
		:
		m_Desc(desc)
	{
		for (auto format : m_Desc.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(format.TextureFormat))
				m_ColorAttachmentSpecifications.emplace_back(format);
			else
				m_DepthAttachmentSpecification = format;
		}

		Recreate();
	}

	DirectXFramebuffer::~DirectXFramebuffer()
	{
		
	}

	static std::tuple<DXGI_FORMAT> GetD3DSpecs(FramebufferTextureSpecification spec)
	{
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		switch (spec.TextureFormat)
		{
		case FramebufferTextureFormat::RGBA8:
			format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case FramebufferTextureFormat::RINT:
			format = DXGI_FORMAT_R32_SINT;
			break;
		case FramebufferTextureFormat::DEPTH32F:
			format = DXGI_FORMAT_D32_FLOAT;
			break;
		case FramebufferTextureFormat::DEPTH24STENCIL8:
			format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		}

		return { format };
	}

	void DirectXFramebuffer::Recreate()
	{
		Invalidate();

		bool multisample = m_Desc.Samples > 1;
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachmentTextures.resize(m_ColorAttachmentSpecifications.size());
			m_ColorAttachmentRenderTargets.resize(m_ColorAttachmentSpecifications.size());
			m_ColorAttachmentSRVs.resize(m_ColorAttachmentSpecifications.size());
			
			for (int i = 0; i < m_ColorAttachmentSpecifications.size(); i++)
			{
				auto [format] = GetD3DSpecs(m_ColorAttachmentSpecifications[i]);

				Utils::CreateRenderTarget(&m_ColorAttachmentTextures[i], &m_ColorAttachmentRenderTargets[i], &m_ColorAttachmentSRVs[i], format, m_Desc.Samples, m_Desc.Width, m_Desc.Height);
			}
		}

		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
		{
			auto [format] = GetD3DSpecs(m_DepthAttachmentSpecification);

			Utils::CreateDepthTexture(m_DepthAttachment.pDepthStencilView.GetAddressOf(), m_DepthAttachment.pDSState.GetAddressOf(), format, m_Desc.Width, m_Desc.Height);
		}

		//Configure viewport
		vp.Width = (FLOAT)m_Desc.Width;
		vp.Height = (FLOAT)m_Desc.Height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
	}

	void* DirectXFramebuffer::GetRenderTextureID(uint32_t index)
	{
		return (void*)m_ColorAttachmentSRVs[index];
	}

	void DirectXFramebuffer::Clear()
	{
		ID3D11DeviceContext* context = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext();
		for (int i = 0; i < m_ColorAttachmentRenderTargets.size(); i++)
		{
			// Clear the back buffer.
			context->ClearRenderTargetView(m_ColorAttachmentRenderTargets[i], m_Desc.ClearColor);
		}

		// Clear the depth buffer.
		context->ClearDepthStencilView(m_DepthAttachment.pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		context->RSSetViewports(1, &vp);
	}

	void DirectXFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Desc.Width = width;
		m_Desc.Height = height;

		Recreate();
	}

	void* DirectXFramebuffer::ReadPixel_Impl(uint32_t targetIndex, int x, int y, uint32_t size)
	{
		assert(targetIndex < m_ColorAttachmentTextures.size());
		//assert(m_ColorAttachmentSpecifications[attachmentIndex].TextureFormat == FramebufferTextureFormat::RINT);

		ID3D11Texture2D* texture = m_ColorAttachmentTextures[targetIndex];
		ID3D11DeviceContext* context = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext();
		ID3D11Device* device = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice();

		D3D11_TEXTURE2D_DESC textureDesc;
		texture->GetDesc(&textureDesc);

		Microsoft::WRL::ComPtr<ID3D11Texture2D> copyTexture;

		D3D11_TEXTURE2D_DESC copyTextureDesc = {};
		copyTextureDesc.Width = textureDesc.Width;
		copyTextureDesc.Height = textureDesc.Height;
		copyTextureDesc.MipLevels = 1;
		copyTextureDesc.ArraySize = 1;
		copyTextureDesc.BindFlags = 0;
		copyTextureDesc.SampleDesc.Count = 1;
		copyTextureDesc.SampleDesc.Quality = 0;
		copyTextureDesc.Usage = D3D11_USAGE_STAGING;
		copyTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		copyTextureDesc.Format = textureDesc.Format;

		// Create the render target texture.
		GFX_THROW_INFO(device->CreateTexture2D(&copyTextureDesc, NULL, &copyTexture));

		context->CopyResource(copyTexture.Get(), texture);

		D3D11_MAPPED_SUBRESOURCE msr;
		GFX_THROW_INFO(
			context->Map(
				copyTexture.Get(), 0, D3D11_MAP_READ, 0, &msr
			)
		);

		void* pixel = malloc(size);
		int pos = x * size + y * msr.RowPitch;
		memcpy(pixel, ((char*)msr.pData) + pos, size);

		context->Unmap(copyTexture.Get(), 0);
		return pixel;
	}

	void DirectXFramebuffer::Bind()
	{
		if (m_DepthAttachment.pDSState)
		{
			//Bind depth stencil state
			((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetDepthStencilState(m_DepthAttachment.pDSState.Get(), 1);
		}

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetRenderTargets((UINT)m_ColorAttachmentRenderTargets.size(), m_ColorAttachmentRenderTargets.data(), m_DepthAttachment.pDepthStencilView.Get());
		
		Clear();
	}

	void DirectXFramebuffer::Invalidate()
	{
		if (m_ColorAttachmentTextures.size())
		{
			for (int i = 0; i < m_ColorAttachmentTextures.size(); i++)
			{
				m_ColorAttachmentTextures[i]->Release();
				m_ColorAttachmentRenderTargets[i]->Release();
				m_ColorAttachmentSRVs[i]->Release();
			}

			if (m_DepthAttachment)
			{
				m_DepthAttachment.pDepthStencilView->Release();
				m_DepthAttachment.pDSState->Release();
			}

			m_ColorAttachmentTextures.clear();
			m_ColorAttachmentRenderTargets.clear();
			m_ColorAttachmentSRVs.clear();
		}
	}

	/*void DirectXFramebuffer::Unbind()
	{
		//Unbind depth stencil state
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetDepthStencilState(nullptr, 0);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
	}*/
}