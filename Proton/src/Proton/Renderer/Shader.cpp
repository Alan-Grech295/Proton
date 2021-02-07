#include "ptpch.h"
#include "Shader.h"
#include <cassert>
#include "Renderer.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXShader.h"
#endif

namespace Proton
{
	PixelShader* PixelShader::Create(std::string path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXPixelShader(path);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}

	VertexShader* VertexShader::Create(std::string path)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXVertexShader(path);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}
}