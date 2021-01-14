#pragma once
#include "ptpch.h"
#include <d3d11.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include "WindowsWindow.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Proton
{
	class WindowsGraphics
	{
	public:
		WindowsGraphics(HWND hWnd, UINT width, UINT height);
		WindowsGraphics(const WindowsGraphics&) = delete;
		WindowsGraphics& operator=(const WindowsGraphics&) = delete;
		~WindowsGraphics() = default;

		void ShowFrame();
		void ClearBuffer(float r, float g, float b);

		void DrawTestTriangle();

		void SetVSync(bool enabled);
	private:
		std::wstring s2ws(const std::string& s);

		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		std::string pixelShaderPath;
		std::string vertexShaderPath;

		UINT width, height;

		bool isVSync;
	};
}