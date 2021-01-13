#include "ptpch.h"
#include "WindowsGraphics.h"
#include <d3dcompiler.h>
#include "Proton/Log.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace wrl = Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Proton
{
	WindowsGraphics::WindowsGraphics(HWND hWnd)
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

		pBackBuffer->Release();
	}

	void WindowsGraphics::ShowFrame()
	{
		pSwap->Present(1, 0);
	}

	void WindowsGraphics::ClearBuffer(float r, float g, float b)
	{
		const float color[] = { r, g, b, 1 };
		pContext->ClearRenderTargetView(pTarget.Get(), color);
	}

	void WindowsGraphics::DrawTestTriangle()
	{
		namespace wrl = Microsoft::WRL;

		struct Vertex
		{
			float x;
			float y;
		};

		//Setting an array with the triangle vertices (2D triangle at the centre of the screen)
		const Vertex vertices[] =
		{
			{0.0f, 0.5f},
			{0.5f, -0.5f},
			{-0.5f, -0.5f}
		};

		wrl::ComPtr<ID3D11Buffer> pVertexBuffer;
		//Setting the vertex buffer description
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.ByteWidth = (sizeof(vertices));
		bd.StructureByteStride = 0;

		//Setting the vertex data to the buffer
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;

		//Creating the vertex buffer
		pDevice->CreateBuffer(&bd, &sd, &pVertexBuffer);

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0;

		//Binding vertex buffer to pipeline
		pContext->IASetVertexBuffers(0, 1, pVertexBuffer.GetAddressOf(), &stride, &offset);

		//TODO: Create automatic path

		wrl::ComPtr<ID3DBlob> pBlob;

		//Create pixel shader
		wrl::ComPtr<ID3D11PixelShader> pPixelShader;
		D3DReadFileToBlob(L"C:/Dev/Proton/Proton/PixelShader.cso", &pBlob);
		pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);

		//Bind pixel shader
		pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);

		//Create vertex shader
		wrl::ComPtr<ID3D11VertexShader> pVertexShader;

		D3DReadFileToBlob(L"C:/Dev/Proton/Proton/VertexShader.cso", &pBlob);
		pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);

		//Bind vertex shader
		pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

		//Input (vertex) layout (2D position only)
		wrl::ComPtr<ID3D11InputLayout> pInputLayout;
		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		pDevice->CreateInputLayout(
			ied,
			std::size(ied),
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			&pInputLayout
			);

		//Bind input layout
		pContext->IASetInputLayout(pInputLayout.Get());

		//Bind render target
		pContext->OMSetRenderTargets(1, pTarget.GetAddressOf(), nullptr);

		//Set primitive topology to triangle list (group of 3 vertices)
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//Configure viewport
		D3D11_VIEWPORT vp;
		vp.Width = 1280;
		vp.Height = 720;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		pContext->RSSetViewports(1, &vp);

		pContext->Draw(std::size(vertices), 0);
	}
}