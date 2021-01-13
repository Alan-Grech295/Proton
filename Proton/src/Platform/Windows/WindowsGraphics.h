#pragma once
#include "ptpch.h"
#include <d3d11.h>

namespace Proton
{
	class WindowsGraphics
	{
	public:
		WindowsGraphics(HWND hWnd);
		WindowsGraphics(const WindowsGraphics&) = delete;
		WindowsGraphics& operator=(const WindowsGraphics&) = delete;
		~WindowsGraphics();

		void ShowFrame();
		void ClearBuffer(float r, float g, float b);
	private:
		ID3D11Device* pDevice = nullptr;
		IDXGISwapChain* pSwap = nullptr;
		ID3D11DeviceContext* pContext = nullptr;
		ID3D11RenderTargetView* pTarget = nullptr;
	};
}