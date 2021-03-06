#pragma once
#include "Proton\Renderer\RendererAPI.h"
#include <wrl.h>
#include <d3d11.h>
#include "Proton\Core\Log.h"

namespace Proton
{
	class DirectXRendererAPI : public RendererAPI
	{
		friend class WindowsWindow;
	public:
		virtual void SetClearColor(float r, float g, float b) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const UINT count) override;

		ID3D11Device* GetDevice() { return pDevice.Get(); }
		ID3D11DeviceContext* GetContext() { return pContext.Get(); }
		ID3D11RenderTargetView* GetRenderTarget() { return pTarget.Get(); }
		void SetRenderTarget(ID3D11RenderTargetView* target) { pTarget = target; pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDSV.Get());}
	private:
		void Initialize(WindowsWindow& window, HWND hWnd);
		void ShowFrame();
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;

		UINT width, height;

		bool isVSync;

		float* clearColor;
	};
}