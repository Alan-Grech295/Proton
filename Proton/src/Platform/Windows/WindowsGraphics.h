#pragma once
#include "ptpch.h"
#include <d3d11.h>
#include <wrl.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Proton
{
	class WindowsGraphics
	{
	public:
		WindowsGraphics(HWND hWnd);
		WindowsGraphics(const WindowsGraphics&) = delete;
		WindowsGraphics& operator=(const WindowsGraphics&) = delete;
		~WindowsGraphics() = default;

		void ShowFrame();
		void ClearBuffer(float r, float g, float b);

		void DrawTestTriangle();
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
	};
}