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

		vertexBuffer->Allocate(numElements * layout.stride);
		
		return vertexBuffer;
	}

	Scope<VertexBuffer> VertexBuffer::CreateUnique(const BufferLayout& layout, VertexShader* vertexShader, uint32_t numElements)
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

		vertexBuffer->Allocate(numElements * layout.stride);

		return vertexBuffer;
	}

	Scope<VertexBuffer> VertexBuffer::CreateUnique(Ref<Bindable> other)
	{
		assert("Test first before using!" && false);
		VertexBuffer& vb = *static_cast<VertexBuffer*>(other.get());
		return VertexBuffer::CreateUnique(const_cast<BufferLayout&>(vb.GetLayout()), vb.m_Shader, vb.size());
	}

	Ref<VertexBuffer> VertexBuffer::Clone(const VertexBuffer& other)
	{
		Ref<VertexBuffer> clone = CreateUnique(other.m_Layout, other.m_Shader, other.size());
		clone->SetRawData(other.m_Data, other.sizeBytes());
		return clone;
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
		return IndexBuffer::CreateUnique((uint32_t)static_cast<IndexBuffer*>(other.get())->m_Indices.size());
	}

	Ref<IndexBuffer> IndexBuffer::Clone(const IndexBuffer& other)
	{
		Ref<IndexBuffer> clone = std::move(CreateUnique(other.size()));
		clone->SetRawData(other.m_Indices.data(), other.m_Indices.size());
		return clone;
	}

	void VertexConstantBuffer::SetData(const void* data, int size)
	{
		if (size < 0) size = m_Size;

		memcpy(m_Data, data, size);
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
		return CreateUnique(other.get());
	}

	Scope<VertexConstantBuffer> VertexConstantBuffer::CreateUnique(Bindable* other)
	{
		VertexConstantBuffer* vcb = static_cast<VertexConstantBuffer*>(other);
		void* data = vcb->GetData();
		Scope< VertexConstantBuffer> result = VertexConstantBuffer::CreateUnique(vcb->m_Slot, vcb->m_Size, data);
		return result;
	}

	Ref<VertexConstantBuffer> VertexConstantBuffer::Clone(const VertexConstantBuffer& other)
	{
		Ref<VertexConstantBuffer> clone = std::move(CreateUnique(other.m_Slot, other.m_Size, other.GetData()));
		return clone;
	}

	Ref<PixelConstantBuffer> PixelConstantBuffer::Create(const std::string& tag, int slot, DCB::CookedLayout& layout)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXPixelConstantBuffer>(tag, slot, layout);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<PixelConstantBuffer> PixelConstantBuffer::CreateUnique(int slot, const DCB::CookedLayout& layout)
	{
		PT_PROFILE_FUNCTION();

		//assert("Size must be greater than or equal to 16!" && size >= 16);

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXPixelConstantBuffer>("", slot, layout);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<PixelConstantBuffer> PixelConstantBuffer::CreateUnique(Ref<Bindable> other)
	{
		return CreateUnique(other.get());
	}

	Scope<PixelConstantBuffer> PixelConstantBuffer::CreateUnique(Bindable* other)
	{
		PixelConstantBuffer* pcb = static_cast<PixelConstantBuffer*>(other);
		DCB::CookedLayout cookedLayout = DCB::CookedLayout(pcb->m_Root);
		Scope<PixelConstantBuffer> result = PixelConstantBuffer::CreateUnique(pcb->m_Slot, cookedLayout);
		memcpy(result->m_Data, pcb->GetData(), pcb->m_Size);
		return result;
	}

	Ref<PixelConstantBuffer> PixelConstantBuffer::Clone(const PixelConstantBuffer& other)
	{
		Ref<PixelConstantBuffer> clone = std::move(CreateUnique(other.m_Slot, DCB::CookedLayout(other.m_Root)));
		clone->m_Size = other.m_Size;
		memcpy(clone->m_Data, other.m_Data, other.m_Size);
		clone->m_Changed = true;
		return clone;
	}

	PixelConstantBuffer* PixelConstantBuffer::CreateUniquePtr(int slot, const DCB::CookedLayout& layout)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXPixelConstantBuffer("", slot, layout);
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