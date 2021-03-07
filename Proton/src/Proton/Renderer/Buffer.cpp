#include "ptpch.h"
#include "Buffer.h"
#include <cassert>

#include "Renderer.h"
#include "BindsCollection.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXBuffer.h"
#endif

namespace Proton
{
	Ref<VertexBuffer> VertexBuffer::Create(const std::string& tag, int stride, const void* vertices, uint32_t size)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXVertexBuffer>(tag, stride, vertices, size);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<VertexBuffer> VertexBuffer::CreateUnique(int stride, const void* vertices, uint32_t size)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXVertexBuffer>("", stride, vertices, size);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::string& tag, unsigned short* indices, uint32_t size)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXIndexBuffer>(tag, indices, size);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<IndexBuffer> IndexBuffer::CreateUnique(unsigned short* indices, uint32_t size)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXIndexBuffer>("", indices, size);
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
}