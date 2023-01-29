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
	Ref<VertexBuffer> VertexBuffer::Create(const std::string& tag, BufferLayout& layout, VertexShader* vertexShader, uint32_t numElements)
	{
		PT_PROFILE_FUNCTION();

		Ref<VertexBuffer> vertexBuffer;

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			vertexBuffer = BindsCollection::Resolve<DirectXVertexBuffer>(tag, layout, vertexShader); break;
		default:
			assert("Unknown RendererAPI!");
		}

		vertexBuffer->m_Data = (char*)malloc(numElements * layout.stride);
		vertexBuffer->m_End = vertexBuffer->m_Data;
		vertexBuffer->m_MaxSize = numElements * layout.stride;
		
		return vertexBuffer;
	}

	Scope<VertexBuffer> VertexBuffer::CreateUnique(BufferLayout& layout, VertexShader* vertexShader, uint32_t numElements)
	{
		PT_PROFILE_FUNCTION();

		Scope<VertexBuffer> vertexBuffer;

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			vertexBuffer = CreateScope<DirectXVertexBuffer>("", layout, vertexShader); break;
		default:
			assert("Unknown RendererAPI!");
		}

		vertexBuffer->m_Data = (char*)malloc(numElements * layout.stride);
		vertexBuffer->m_End = vertexBuffer->m_Data;
		vertexBuffer->m_MaxSize = numElements * layout.stride;

		return vertexBuffer;
	}

	Scope<VertexBuffer> VertexBuffer::CreateUnique(Ref<Bindable> other)
	{
		assert("Test first before using!" && false);
		VertexBuffer& vb = dynamic_cast<VertexBuffer&>(*other);
		return VertexBuffer::CreateUnique(const_cast<BufferLayout&>(vb.GetLayout()), vb.m_Shader, vb.m_MaxSize);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const std::string& tag, uint32_t size)
	{
		PT_PROFILE_FUNCTION();

		Ref<IndexBuffer> buffer;

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			buffer = BindsCollection::Resolve<DirectXIndexBuffer>(tag); break;
		default:
			assert("Unknown RendererAPI!");
		}

		buffer->m_Indices.reserve(size);
		return buffer;
	}

	Scope<IndexBuffer> IndexBuffer::CreateUnique(uint32_t size)
	{
		PT_PROFILE_FUNCTION();

		Scope<IndexBuffer> buffer;

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			buffer = CreateScope<DirectXIndexBuffer>(""); break;
		default:
			assert("Unknown RendererAPI!");
		}

		buffer->m_Indices.reserve(size);
		return buffer;
	}

	Scope<IndexBuffer> IndexBuffer::CreateUnique(Ref<Bindable> other)
	{
		assert("Test first before using!" && false);
		return IndexBuffer::CreateUnique(dynamic_cast<IndexBuffer*>(other.get())->m_Indices.size());
	}
	
	Ref<VertexConstantBuffer> VertexConstantBuffer::Create(const std::string& tag, int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		assert("Size must be greater than or equal to 16!" && size >= 16);

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

		assert("Size must be greater than or equal to 16!" && size >= 16);

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

	Scope<VertexConstantBuffer> VertexConstantBuffer::CreateUnique(Ref<Bindable> other)
	{
		VertexConstantBuffer* vcb = dynamic_cast<VertexConstantBuffer*>(other.get());
		void* data = vcb->GetData();
		Scope< VertexConstantBuffer> result = VertexConstantBuffer::CreateUnique(vcb->m_Slot, vcb->m_Size, data);
		return result;
	}

	Ref<PixelConstantBuffer> PixelConstantBuffer::Create(const std::string& tag, int slot, int size, const void* data)
	{
		PT_PROFILE_FUNCTION();

		assert("Size must be greater than or equal to 16!" && size >= 16);

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

		assert("Size must be greater than or equal to 16!" && size >= 16);

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

	Scope<PixelConstantBuffer> PixelConstantBuffer::CreateUnique(Ref<Bindable> other)
	{
		PixelConstantBuffer& pcb = dynamic_cast<PixelConstantBuffer&>(*other);
		void* data = pcb.GetData();
		Scope<PixelConstantBuffer> result = PixelConstantBuffer::CreateUnique(pcb.m_Slot, pcb.m_Size, data);
		return result;
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