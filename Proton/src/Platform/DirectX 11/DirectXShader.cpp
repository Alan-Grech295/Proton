#include "ptpch.h"
#include "Proton\Log.h"
#include "DirectXShader.h"

namespace Proton
{
	DirectXPixelShader::DirectXPixelShader(std::string path)
	{
		std::wstring wpath = std::wstring(path.begin(), path.end());
		D3DReadFileToBlob(wpath.c_str(), &pBytecodeBlob);
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreatePixelShader(pBytecodeBlob->GetBufferPointer(), pBytecodeBlob->GetBufferSize(), nullptr, &pPixelShader);
	}

	void DirectXPixelShader::Bind() const
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	}

	void DirectXPixelShader::Unbind() const
	{

	}

	DirectXVertexShader::DirectXVertexShader(std::string path)
	{
		std::wstring wpath = std::wstring(path.begin(), path.end());
		D3DReadFileToBlob(wpath.c_str(), &pBytecodeBlob);
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateVertexShader(pBytecodeBlob->GetBufferPointer(), pBytecodeBlob->GetBufferSize(), nullptr, &pVertexShader);
	}

	void DirectXVertexShader::Bind() const
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	}

	void DirectXVertexShader::Unbind() const
	{

	}
}

