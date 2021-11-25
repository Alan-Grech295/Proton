#include "ptpch.h"
#include "Buffer.h"
#include <cassert>

#include "..\Renderer.h"
#include "BindsCollection.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXBuffer.h"
#endif

namespace Proton
{
	Ref<VertexBuffer> VertexBuffer::Create(const std::string& tag, BufferLayout& layout, VertexShader* vertexShader)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXVertexBuffer>(tag, layout, vertexShader);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<VertexBuffer> VertexBuffer::CreateUnique(BufferLayout& layout, VertexShader* vertexShader)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXVertexBuffer>("", layout, vertexShader);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::string& tag)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXIndexBuffer>(tag);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<IndexBuffer> IndexBuffer::CreateUnique()
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXIndexBuffer>("");
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}
	
	Ref<VertexConstantBuffer> VertexConstantBuffer::Create(const std::string& tag, int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXVertexConstantBuffer>(tag, slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<VertexConstantBuffer> VertexConstantBuffer::CreateUnique(int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXVertexConstantBuffer>("", slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Ref<PixelConstantBuffer> PixelConstantBuffer::Create(const std::string& tag, int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXPixelConstantBuffer>(tag, slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<PixelConstantBuffer> PixelConstantBuffer::CreateUnique(int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXPixelConstantBuffer>("", slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	PixelConstantBuffer* PixelConstantBuffer::CreateUniquePtr(int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXPixelConstantBuffer("", slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}
	/*Ref<VertexBuffer> VertexBuffer::Create(BufferLayout& layout, VertexShader* vertexShader, const std::string& tag)
	{
		//return BindsCollection::Resolve<VertexBuffer>(layout, vertexShader, tag);
		return CreateRef<VertexBuffer>(layout, vertexShader, tag);
	}*/
}