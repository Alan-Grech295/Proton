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

		pBackBuffer->Release();

		//Creating Pixel and Vertex shader path strings
		std::string filePath = __FILE__;

		std::string currentWord;
		std::vector<std::string> splitPath;

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

		std::string pathToShaders;

		//Getting the path to the shaders, relative to the current file path
		for (int i = 0; i < splitPath.size() - 4; i++)
		{
			pathToShaders += splitPath[i] + "\\";
		}

		//Getting the paths for the pixel and vertex shaders
		pixelShaderPath = pathToShaders + "PixelShader.cso";
		vertexShaderPath = pathToShaders + "VertexShader.cso";
	}

	void WindowsGraphics::ShowFrame()
	{
		pSwap->Present(isVSync ? 1 : 0, 0);
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
			struct
			{
				float x;
				float y;
			} pos;
			
			struct
			{
				BYTE r;
				BYTE g;
				BYTE b;
				BYTE a;
			} colour;
		};

		//Setting an array with the triangle vertices (2D triangle at the centre of the screen)
		Vertex vertices[] =
		{
			{0.0f, 0.5f, 255, 0, 0, 0},
			{0.5f, -0.5f, 0, 255, 0, 0},
			{-0.5f, -0.5f, 0, 0, 255, 0},
			{-0.3f, 0.3f, 0, 255, 0, 0},
			{0.3f, 0.3f, 0, 0, 255, 0},
			{0.0f, -0.8f, 255, 0, 0, 0}
		};

		vertices[0].colour.g = 255;

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

		//Create index buffer
		unsigned short indices[] = 
		{
			0, 1, 2,
			0, 2, 3,
			0, 4, 1,
			2, 1, 5
		};

		wrl::ComPtr<ID3D11Buffer> pIndexBuffer;

		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.ByteWidth = (sizeof(indices));
		ibd.StructureByteStride = 0;

		//Setting the index data to the buffer
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = indices;

		//Creating the index buffer
		pDevice->CreateBuffer(&ibd, &isd, &pIndexBuffer);

		//Binding index buffer to pipeline
		pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

		wrl::ComPtr<ID3DBlob> pBlob;

		//Create pixel shader
		wrl::ComPtr<ID3D11PixelShader> pPixelShader;
		D3DReadFileToBlob(s2ws(pixelShaderPath).c_str(), &pBlob);
		pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader);

		//Bind pixel shader
		pContext->PSSetShader(pPixelShader.Get(), nullptr, 0);

		//Create vertex shader
		wrl::ComPtr<ID3D11VertexShader> pVertexShader;

		D3DReadFileToBlob(s2ws(vertexShaderPath).c_str(), &pBlob);
		pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader);

		//Bind vertex shader
		pContext->VSSetShader(pVertexShader.Get(), nullptr, 0);

		//Input (vertex) layout (2D position only)
		wrl::ComPtr<ID3D11InputLayout> pInputLayout;
		const D3D11_INPUT_ELEMENT_DESC ied[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOUR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
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
		vp.Width = width;
		vp.Height = height;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;

		pContext->RSSetViewports(1, &vp);

		pContext->DrawIndexed(std::size(indices), 0, 0);
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
}