#pragma once
#include "ptpch.h"
#include <d3d11.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "WindowsWindow.h"
#include <random>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Proton
{
	class Box;

	class WindowsGraphics
	{
		friend class Bindable;
	public:
		WindowsGraphics(HWND hWnd, UINT width, UINT height);
		WindowsGraphics(const WindowsGraphics&) = delete;
		WindowsGraphics& operator=(const WindowsGraphics&) = delete;
		~WindowsGraphics() = default;

		void ShowFrame();
		void ClearBuffer(float r, float g, float b);

		void SetVSync(bool enabled);

		void DrawIndexed(UINT count) noexcept;
		void SetProjection(DirectX::FXMMATRIX proj) noexcept;
		DirectX::XMMATRIX GetProjection() const noexcept;

		std::wstring GetShaderPath(std::string shaderName);
	private:
		std::wstring s2ws(const std::string& s);

		DirectX::XMMATRIX projection;

		Microsoft::WRL::ComPtr<ID3D11Device> pDevice;
		Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> pContext;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTarget;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDSV;

		std::string absPath;

		UINT width, height;

		bool isVSync;
	};
}