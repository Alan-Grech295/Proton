#pragma once
#include "Proton\Renderer\RendererAPI.h"
#include <wrl.h>
#include <d3d11.h>
#include "Proton\Core\Log.h"

namespace Proton
{
	class DirectXRendererAPI : public RendererAPI
	{
	public:
		virtual void SetClearColor(float r, float g, float b) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const UINT count) override;
		virtual void Draw(const UINT count) override;

		virtual void SetTopology(const TopologyType topology) override;

		virtual void SetVsync(bool vsync) override { isVSync = vsync; }

		ID3D11Device* GetDevice() { return pDevice.Get(); }
		ID3D11DeviceContext* GetContext() { return pContext.Get(); }
		ID3D11RenderTargetView* GetRenderTarget() { return pTarget.Get(); }
		void SetRenderTarget(ID3D11RenderTargetView* target) { pTarget = target; pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDSV.Get());}

		void BindSwapChain() override;

		virtual void Initialise(const Window& window) override;

		virtual void Present() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
		D3D11_VIEWPORT vp;

		UINT m_Width, m_Height;

		bool isVSync;

		float* clearColor;
	};
}