#include "ptpch.h"
#include "DirectXFramebuffer.h"
#include "DirectXRendererAPI.h"
#include "Proton\Renderer\RenderCommand.h"

namespace Proton
{
	DirectXFramebuffer::DirectXFramebuffer(const FramebufferDescription& desc)
		:
		m_Desc(desc)
	{
		Recreate();
	}

	DirectXFramebuffer::~DirectXFramebuffer()
	{
		
	}

	void DirectXFramebuffer::Recreate()
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		HRESULT hr;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

		// Setup the render target texture description.
		textureDesc.Width = m_Desc.Width;
		textureDesc.Height = m_Desc.Height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;

		// Create the render target texture.
		hr = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&textureDesc, NULL, &pTexture);
		if (FAILED(hr))
		{
			LAST_ERROR();
		}

		// Setup the description of the render target view.
		renderTargetViewDesc.Format = textureDesc.Format;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		// Create the render target view.
		hr = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateRenderTargetView(pTexture.Get(), &renderTargetViewDesc, &pRenderTarget);
		if (FAILED(hr))
		{
			LAST_ERROR();
		}

		// Setup the description of the shader resource view.
		shaderResourceViewDesc.Format = textureDesc.Format;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		// Create the shader resource view.
		hr = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateShaderResourceView(pTexture.Get(), &shaderResourceViewDesc, &pTextureView);
		
		//Create depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateDepthStencilState(&dsDesc, &pDSState);

		//Create depth stencil texture
		Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = m_Desc.Width;
		descDepth.Height = m_Desc.Height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

		//Create depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
		descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDepthStencilView);

		if (FAILED(hr))
		{
			LAST_ERROR();
		}

		//Configure viewport
		vp.Width = (FLOAT)m_Desc.Width;
		vp.Height = (FLOAT)m_Desc.Height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
	}

	void* DirectXFramebuffer::GetRenderTextureID()
	{
		return (void*)pTextureView.Get();
	}

	void DirectXFramebuffer::Clear()
	{
		// Clear the back buffer.
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->ClearRenderTargetView(pRenderTarget.Get(), clearCol);

		// Clear the depth buffer.
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->ClearDepthStencilView(pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->RSSetViewports(1, &vp);
	}

	void DirectXFramebuffer::SetClearCol(float r, float g, float b)
	{
		clearCol = new float[4] {r, g, b, 1};
	}

	void DirectXFramebuffer::Resize(uint32_t width, uint32_t height)
	{
		m_Desc.Width = width;
		m_Desc.Height = height;

		Recreate();
	}

	void DirectXFramebuffer::Bind()
	{
		//Bind depth stencil state
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetDepthStencilState(pDSState.Get(), 1);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetRenderTargets(1, pRenderTarget.GetAddressOf(), pDepthStencilView.Get());
	
		Clear();
	}

	void DirectXFramebuffer::Unbind()
	{
		//Unbind depth stencil state
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetDepthStencilState(nullptr, 0);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->OMSetRenderTargets(0, nullptr, nullptr);
	}
}