#include "ptpch.h"
#include "DirectXBuffer.h"
#include "DirectXShader.h"
#include "Proton\Core\Log.h"

#include "Proton\Core\Core.h"

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	static DXGI_FORMAT ShaderDataTypeToDXGIFormat(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:			return DXGI_FORMAT_R32_FLOAT;
		case ShaderDataType::Float2:		return DXGI_FORMAT_R32G32_FLOAT;
		case ShaderDataType::Float3:		return DXGI_FORMAT_R32G32B32_FLOAT;
		case ShaderDataType::Float4:		return DXGI_FORMAT_R32G32B32A32_FLOAT;
		//case ShaderDataType::Matrix3x3:		return DXGI_FORMAT_R32_FLOAT;
		//case ShaderDataType::Matrix4x4:		return DXGI_FORMAT_R32_FLOAT;
		case ShaderDataType::Int:			return DXGI_FORMAT_R32_SINT;
		case ShaderDataType::Int2:			return DXGI_FORMAT_R32G32_SINT;
		case ShaderDataType::Int3:			return DXGI_FORMAT_R32G32B32_SINT;
		case ShaderDataType::Int4:			return DXGI_FORMAT_R32G32B32A32_SINT;
		//case ShaderDataType::Bool:			return DXGI_FORMAT_R32_FLOAT;
		}

		assert("Unknown ShaderDataType!");
		return DXGI_FORMAT_UNKNOWN;
	}
	//Vertex Buffer
	DirectXVertexBuffer::DirectXVertexBuffer(const std::string& tag, BufferLayout& layout, VertexShader* vertexShader)
		:
		m_InputLayoutDesc(new D3D11_INPUT_ELEMENT_DESC[layout.size()])
	{
		m_Uid = tag;
		m_Layout = layout;

		for (int i = 0; i < m_Layout.size(); i++)
		{
			m_InputLayoutDesc[i] = { m_Layout[i].Name.c_str(), 0, ShaderDataTypeToDXGIFormat(m_Layout[i].Type), 0, m_Layout[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		}

		DirectXVertexShader& dxVertexShader = *(DirectXVertexShader*)(vertexShader);

		DX_CHECK_ERROR(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateInputLayout(
			m_InputLayoutDesc, layout.size(),
			dxVertexShader.pBytecodeBlob->GetBufferPointer(),
			dxVertexShader.pBytecodeBlob->GetBufferSize(),
			&m_InputLayout
		));
	}

	void DirectXVertexBuffer::Bind()
	{
		if (m_Changed)
		{
			RecreateBuffer();
			m_Changed = false;
		}

		DirectXRendererAPI* api = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI());

		const static UINT offset = 0u;
		api->GetContext()->IASetVertexBuffers(0, 1u, m_VertexBuffer.GetAddressOf(), &m_Layout.stride, &offset);
		api->GetContext()->IASetInputLayout(m_InputLayout.Get());
	}

	std::string DirectXVertexBuffer::GetUID() const noexcept
	{
		return m_Uid;
	}

	void DirectXVertexBuffer::RecreateBuffer()
	{
		if (m_PastBufferSize == size())
		{
			D3D11_MAPPED_SUBRESOURCE msr;
			ZeroMemory(&msr, sizeof(D3D11_MAPPED_SUBRESOURCE));
			DX_CHECK_ERROR(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
				m_VertexBuffer.Get(), 0,
				D3D11_MAP_WRITE_DISCARD, 0,
				&msr
			));

			memcpy(msr.pData, m_Data, size());

			((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Unmap(m_VertexBuffer.Get(), 0);
		}
		else
		{
			m_PastBufferSize = size();

			D3D11_BUFFER_DESC bd = {};
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			bd.MiscFlags = 0u;
			bd.ByteWidth = size();
			//bd.StructureByteStride = m_Stride;
			D3D11_SUBRESOURCE_DATA sd = {};
			sd.pSysMem = m_Data;

			((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer);
		}
	}

	//Index Buffer
	DirectXIndexBuffer::DirectXIndexBuffer(const std::string& tag)
	{
		m_Uid = tag;
	}

	void DirectXIndexBuffer::Bind()
	{
		if (m_Changed)
		{
			RecreateBuffer();
			m_Changed = false;
		}
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	}

	void DirectXIndexBuffer::RecreateBuffer()
	{
		if (m_PastBufferSize == m_Indices.size())
		{
			D3D11_MAPPED_SUBRESOURCE msr;
			((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
				m_IndexBuffer.Get(), 0,
				D3D11_MAP_WRITE_DISCARD, 0,
				&msr
			);

			memcpy(msr.pData, m_Indices.data(), m_Indices.size() * sizeof(uint32_t));

			((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Unmap(m_IndexBuffer.Get(), 0);
		}
		else
		{
			m_PastBufferSize = m_Indices.size();
			D3D11_BUFFER_DESC ibd = {};
			ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			ibd.Usage = D3D11_USAGE_DEFAULT;
			ibd.CPUAccessFlags = 0u;
			ibd.MiscFlags = 0u;
			ibd.ByteWidth = m_Indices.size() * sizeof(uint32_t);
			//ibd.StructureByteStride = sizeof(unsigned short);
			D3D11_SUBRESOURCE_DATA isd = {};
			isd.pSysMem = m_Indices.data();
			((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&ibd, &isd, &m_IndexBuffer);
		}
	}

	//Vertex Constant Buffer
	DirectXVertexConstantBuffer::DirectXVertexConstantBuffer(const std::string& tag, int slot, int size, const void* data)
		:
		uid(tag)
	{
		m_Slot = slot;
		m_Size = size;

		m_Data = malloc(size);
		if (data)
			memcpy(m_Data, data, size);
		
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.ByteWidth = size;
		cbd.MiscFlags = 0u;
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = m_Data;
		csd.SysMemPitch = 0;
		csd.SysMemSlicePitch = 0;

		DX_CHECK_ERROR(((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&cbd, data ? &csd : nullptr, &pConstantBuffer));
	}

	void DirectXVertexConstantBuffer::SetData(const void* data)
	{
		memcpy(m_Data, data, m_Size);

		D3D11_MAPPED_SUBRESOURCE msr;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
			pConstantBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0,
			&msr
		);

		memcpy(msr.pData, data, m_Size);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Unmap(pConstantBuffer.Get(), 0);
	}

	/*IMPORTANT: Deallocate the data as it is a copy*/
	void* DirectXVertexConstantBuffer::GetData()
	{
		return m_Data;
	}

	void DirectXVertexConstantBuffer::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->VSSetConstantBuffers(m_Slot, 1, pConstantBuffer.GetAddressOf());
	}

	std::string DirectXVertexConstantBuffer::GetUID() const noexcept
	{
		return uid;
	}

	//Pixel Constant Buffer
	DirectXPixelConstantBuffer::DirectXPixelConstantBuffer(const std::string& tag, int slot, int size, const void* data)
		:
		uid(tag)
	{
		m_Slot = slot;
		m_Size = size;

		m_Data = malloc(size);
		if (data)
			memcpy(m_Data, data, size);

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.ByteWidth = size;
		cbd.MiscFlags = 0u;
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = m_Data;
		csd.SysMemPitch = 0;
		csd.SysMemSlicePitch = 0;

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&cbd, &csd, &pConstantBuffer);
	}

	void DirectXPixelConstantBuffer::SetData(const void* data)
	{
		memcpy(m_Data, data, m_Size);

		D3D11_MAPPED_SUBRESOURCE msr;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
			pConstantBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0,
			&msr
		);

		memcpy(msr.pData, data, m_Size);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Unmap(pConstantBuffer.Get(), 0);
	}

	/*IMPORTANT: Deallocate the data as it is a copy*/
	void* DirectXPixelConstantBuffer::GetData()
	{
		return m_Data;
	}

	void DirectXPixelConstantBuffer::Bind()
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetConstantBuffers(m_Slot, 1, pConstantBuffer.GetAddressOf());
	}

	std::string DirectXPixelConstantBuffer::GetUID() const noexcept
	{
		return uid;
	}
}