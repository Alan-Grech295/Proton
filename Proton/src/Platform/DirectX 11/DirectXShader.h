#pragma once
#include "Proton\Renderer\Shader.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"
#include "Proton\Renderer\RenderCommand.h"
#include <wrl.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

namespace Proton
{
	class DirectXPixelShader : public PixelShader
	{
	public:
		DirectXPixelShader(std::string path);
		virtual ~DirectXPixelShader() {}

		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	};

	class DirectXVertexShader : public VertexShader
	{
		friend class DirectXVertexBuffer;
	public:
		DirectXVertexShader(std::string path);
		virtual ~DirectXVertexShader() {}

		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		Microsoft::WRL::ComPtr<ID3DBlob> pBytecodeBlob;
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	};
}