#include "ptpch.h"
#include "Proton\Core\Log.h"
#include "DirectXShader.h"

namespace Proton
{
	DirectXPixelShader::DirectXPixelShader(std::string path)
		:
		m_Path(path)
	{
		std::wstring wpath = std::wstring(path.begin(), path.end());
		D3DReadFileToBlob(wpath.c_str(), &pBytecodeBlob);
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreatePixelShader(pBytecodeBlob->GetBufferPointer(), pBytecodeBlob->GetBufferSize(), nullptr, &pPixelShader);
	}

	void DirectXPixelShader::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetShader(pPixelShader.Get(), nullptr, 0u);
	}

	std::string DirectXPixelShader::GetUID() const noexcept
	{
		return m_Path;
	}

	DirectXVertexShader::DirectXVertexShader(std::string path)
		:
		m_Path(path)
	{
		std::wstring wpath = std::wstring(path.begin(), path.end());
		D3DReadFileToBlob(wpath.c_str(), &pBytecodeBlob);
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateVertexShader(pBytecodeBlob->GetBufferPointer(), pBytecodeBlob->GetBufferSize(), nullptr, &pVertexShader);
	}

	void DirectXVertexShader::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->VSSetShader(pVertexShader.Get(), nullptr, 0u);
	}

	std::string DirectXVertexShader::GetUID() const noexcept
	{
		return m_Path;
	}
}

