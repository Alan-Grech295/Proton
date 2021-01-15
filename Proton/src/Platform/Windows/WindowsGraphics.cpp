#include "ptpch.h"
#include "WindowsGraphics.h"
#include <d3dcompiler.h>
#include "Proton/Log.h"
#include <filesystem>
#include <DirectXMath.h>

namespace fs = std::filesystem;

namespace wrl = Microsoft::WRL;

namespace dx = DirectX;

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
		pContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void WindowsGraphics::DrawTestCube(float angle, float x, float z)
	{
		namespace wrl = Microsoft::WRL;

		struct Vertex
		{
			struct
			{
				float x;
				float y;
				float z;
			} pos;
		};

		//Setting an array with the triangle vertices (2D triangle at the centre of the screen)
		Vertex vertices[] =
		{
			{-1.0f, -1.0f, -1.0f},
			{1.0f, -1.0f, -1.0f},
			{-1.0f, 1.0f, -1.0f},
			{1.0f, 1.0f, -1.0f},
			{-1.0f, -1.0f, 1.0f},
			{1.0f, -1.0f, 1.0f},
			{-1.0f, 1.0f, 1.0f},
			{1.0f, 1.0f, 1.0f},
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

		//Create index buffer
		unsigned short indices[] = 
		{
			0, 2, 1,  2, 3, 1,
			1, 3, 5,  3, 7, 5,
			2, 6, 3,  3, 6, 7,
			4, 5, 7,  4, 7, 6,
			0, 4, 2,  2, 4, 6,
			0, 1, 4,  1, 5, 4
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

		//Create constant buffer for transformation matrix
		struct ConstantBuffer
		{
			dx::XMMATRIX transform;
		};

		const ConstantBuffer cb =
		{
				dx::XMMatrixTranspose(
					dx::XMMatrixRotationX(angle) *
					dx::XMMatrixRotationY(angle) *
					dx::XMMatrixRotationZ(angle) *
					dx::XMMatrixTranslation(x, 0, z + 4.0f) *
					dx::XMMatrixPerspectiveLH(1.0f, (float)height / width, 0.5f, 10.0f)
				)
		};

		//Create the constant buffer
		wrl::ComPtr<ID3D11Buffer> pConstantBuffer;
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0;
		cbd.ByteWidth = sizeof(cb);
		cbd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &cb;

		pDevice->CreateBuffer(&cbd, &csd, &pConstantBuffer);

		//Bind constant buffer to vertex shader
		pContext->VSSetConstantBuffers(0, 1, pConstantBuffer.GetAddressOf());

		struct ConstantBuffer2
		{
			struct
			{
				float r;
				float g;
				float b;
				float a;
			} face_colours[6];
		};

		const ConstantBuffer2 cb2 =
		{
			{
				{1.0f, 0.0f, 1.0f},
				{1.0f, 0.0f, 0.0f},
				{0.0f, 1.0f, 0.0f},
				{0.0f, 0.0f, 1.0f},
				{1.0f, 1.0f, 0.0f},
				{0.0f, 1.0f, 1.0f},
			}
		};

		//Create the constant buffer
		wrl::ComPtr<ID3D11Buffer> pConstantBuffer2;
		D3D11_BUFFER_DESC cbd2;
		cbd2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd2.Usage = D3D11_USAGE_DYNAMIC;
		cbd2.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd2.MiscFlags = 0;
		cbd2.ByteWidth = sizeof(cb2);
		cbd2.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA csd2 = {};
		csd2.pSysMem = &cb2;

		pDevice->CreateBuffer(&cbd2, &csd2, &pConstantBuffer2);

		//Bind constant buffer to vertex shader
		pContext->PSSetConstantBuffers(0, 1, pConstantBuffer2.GetAddressOf());

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
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
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