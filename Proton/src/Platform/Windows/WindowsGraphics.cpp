#include "ptpch.h"
#include "WindowsGraphics.h"
#include <d3dcompiler.h>
#include "Proton/Log.h"
#include <filesystem>
#include <DirectXMath.h>
#include <random>
#include "Proton/Box.h"
#include "Platform/DirectX 11/imgui_impl_dx11.h"

namespace fs = std::filesystem;

namespace wrl = Microsoft::WRL;

namespace dx = DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Proton
{
	WindowsGraphics* WindowsGraphics::s_Instance = nullptr;
	std::string WindowsGraphics::absPath = "";

	WindowsGraphics::WindowsGraphics(HWND hWnd, UINT width, UINT height)
		:
		width(width),
		height(height)
	{
		//Swap chain description structure
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;
		sd.BufferDesc.RefreshRate.Denominator = 0;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;
		sd.OutputWindow = hWnd;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		sd.Flags = 0;

		//Create device, front/back buffers, swap chain and rendering context
		D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&sd,
			&pSwap,
			&pDevice,
			nullptr,
			&pContext
		);

		//Gain access to back buffer
		wrl::ComPtr<ID3D11Resource> pBackBuffer;
		pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer);
		pDevice->CreateRenderTargetView(
			pBackBuffer.Get(),
			nullptr,
			&pTarget
		);

		//Create depth stencil state
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		wrl::ComPtr<ID3D11DepthStencilState> pDSState;
		pDevice->CreateDepthStencilState(&dsDesc, &pDSState);

		//Bind depth stencil state
		pContext->OMSetDepthStencilState(pDSState.Get(), 1);

		//Create depth stencil texture
		wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		
		pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

		//Create depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
		descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV);

		pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), pDSV.Get());

		//Configure viewport
		D3D11_VIEWPORT vp;
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		pContext->RSSetViewports(1, &vp);

		//Creating shader absolute paths
		std::string filePath = __FILE__;

		std::string currentWord;
		std::vector<std::string> splitPath;

		//Splitting up the directory by the '\'
		for (int i = 0; i < filePath.length(); i++)
		{
			char c = filePath[i];
			if (c != '\\')
			{
				currentWord += c;
				continue;
			}

			splitPath.push_back(currentWord);
			currentWord = "";
		}

		splitPath.push_back(currentWord);

		//Getting the absolute shader paths
		for (int i = 0; i < splitPath.size() - 4; i++)
		{
			absPath += splitPath[i] + "\\";
		}

		WindowsGraphics::s_Instance = this;
	}

	void WindowsGraphics::ShowFrame()
	{
		pSwap->Present(isVSync ? 1 : 0, 0);
	}

	void WindowsGraphics::ClearBuffer(float r, float g, float b)
	{
		const float color[] = { r, g, b, 1 };
		pContext->ClearRenderTargetView(pTarget.Get(), color);
		pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void WindowsGraphics::SetVSync(bool enabled)
	{
		isVSync = enabled;
	}

	//Converts string to wide string
	std::wstring WindowsGraphics::s2ws(const std::string& s)
	{
		std::wstring stemp = std::wstring(s.begin(), s.end());
		return stemp;
	}

	ID3D11DeviceContext* WindowsGraphics::GetContext()
	{
		return s_Instance->pContext.Get();
	}

	void WindowsGraphics::DrawIndexed(UINT count) noexcept
	{
		pContext->DrawIndexed(count, 0u, 0u);
	}

	void WindowsGraphics::SetProjection(DirectX::FXMMATRIX proj) noexcept
	{
		projection = proj;
	}

	DirectX::XMMATRIX WindowsGraphics::GetProjection() const noexcept
	{
		return projection;
	}

	void WindowsGraphics::SetCamera(DirectX::FXMMATRIX cam)
	{
		this->camera = cam;
	}

	DirectX::XMMATRIX WindowsGraphics::GetCamera() const
	{
		return camera;
	}

	std::string WindowsGraphics::GetShaderPath(std::string shaderName)
	{
		return absPath + shaderName;
	}

	ID3D11Device* WindowsGraphics::GetDevice()
	{
		return s_Instance->pDevice.Get();
	}

	void WindowsGraphics::InitImGui()
	{
		//Init ImGui d3d Impl
		ImGui_ImplDX11_Init(pDevice.Get(), pContext.Get());
	}
}