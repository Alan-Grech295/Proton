#include "ptpch.h"
#include "Buffer.h"
#include <cassert>

#include "Renderer.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXBuffer.h"
#endif

namespace Proton
{
	VertexBuffer* VertexBuffer::Create(int stride, const void* vertices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXVertexBuffer(stride, vertices, size);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(unsigned short* indices, uint32_t size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXIndexBuffer(indices, size);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}
	
	VertexConstantBuffer* VertexConstantBuffer::Create(int slot, int size, const void* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXVertexConstantBuffer(slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	PixelConstantBuffer* PixelConstantBuffer::Create(int slot, int size, const void* data)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXPixelConstantBuffer(slot, size, data);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	PixelConstantBuffer* PixelConstantBuffer::Create(int slot)
	{
		return PixelConstantBuffer::Create(slot, 0, nullptr);
	}

}