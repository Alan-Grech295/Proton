#include "ptpch.h"
#include "Shader.h"
#include <cassert>
#include "Renderer.h"
#include "BindsCollection.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXShader.h"
#endif

namespace Proton
{
	Ref<PixelShader> PixelShader::Create(std::string path)
	{
		PT_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXPixelShader>(path);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}

	Scope<PixelShader> PixelShader::CreateUnique(std::string path)
	{
		PT_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXPixelShader>(path);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexShader> VertexShader::Create(std::string path)
	{
		PT_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXVertexShader>(path);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}

	Scope<VertexShader> VertexShader::CreateUnique(std::string path)
	{
		PT_PROFILE_FUNCTION();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert(false && "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXVertexShader>(path);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}
}