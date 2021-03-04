#include "ptpch.h"
#include "DirectXBuffer.h"
#include "DirectXShader.h"
#include "Proton\Log.h"

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
	DirectXVertexBuffer::DirectXVertexBuffer(const std::string& tag, int stride, const void* vertices, uint32_t size)
		:
		stride(stride),
		uid(tag)
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(stride * size);
		bd.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&bd, &sd, &pVertexBuffer);
	}

	void DirectXVertexBuffer::Bind() const
	{
		DirectXRendererAPI* api = ((DirectXRendererAPI*)RenderCommand::GetRendererAPI());

		const UINT offset = 0u;
		api->GetContext()->IASetVertexBuffers(0, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);
		api->GetContext()->IASetInputLayout(pInputLayout.Get());
	}

	void DirectXVertexBuffer::Unbind() const
	{

	}

	std::string DirectXVertexBuffer::GetUID() const noexcept
	{
		return uid;
	}

	void DirectXVertexBuffer::SetLayout(const BufferLayout& layout, VertexShader* vertexShader)
	{
		m_Layout = layout;
		std::vector<BufferElement> elements = layout.GetElements();
		pInputLayoutDesc = new D3D11_INPUT_ELEMENT_DESC[elements.size()];

		for (int i = 0; i < elements.size(); i++)
		{
			pInputLayoutDesc[i] = { elements[i].Name.c_str(), 0, ShaderDataTypeToDXGIFormat(elements[i].Type), 0, elements[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		}

		DirectXVertexShader& dxVertexShader = *(DirectXVertexShader*)(vertexShader);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateInputLayout(
			pInputLayoutDesc, elements.size(),
			dxVertexShader.pBytecodeBlob->GetBufferPointer(),
			dxVertexShader.pBytecodeBlob->GetBufferSize(),
			&pInputLayout
		);
	}

	//Index Buffer
	DirectXIndexBuffer::DirectXIndexBuffer(const std::string& tag, unsigned short* indices, uint32_t size)
		:
		count(size),
		uid(tag)
	{
		D3D11_BUFFER_DESC ibd = {};
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.CPUAccessFlags = 0u;
		ibd.MiscFlags = 0u;
		ibd.ByteWidth = UINT(count * sizeof(unsigned short));
		ibd.StructureByteStride = sizeof(unsigned short);
		D3D11_SUBRESOURCE_DATA isd = {};
		isd.pSysMem = indices;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&ibd, &isd, &pIndexBuffer);
	}

	void DirectXIndexBuffer::Bind() const
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	}

	void DirectXIndexBuffer::Unbind() const
	{

	}

	std::string DirectXIndexBuffer::GetUID() const noexcept
	{
		return uid;
	}

	unsigned int DirectXIndexBuffer::GetCount() const
	{
		return count;
	}

	//Vertex Constant Buffer
	DirectXVertexConstantBuffer::DirectXVertexConstantBuffer(const std::string& tag, int slot, int size, const void* data)
		:
		mSlot(slot),
		uid(tag)
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = size;
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = data;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&cbd, &csd, &pConstantBuffer);
	}

	void DirectXVertexConstantBuffer::SetData(int size, const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
			pConstantBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0,
			&msr
		);

		memcpy(msr.pData, data, size);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Unmap(pConstantBuffer.Get(), 0);
	}

	void DirectXVertexConstantBuffer::Bind() const
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->VSSetConstantBuffers(mSlot, 1, pConstantBuffer.GetAddressOf());
	}

	void DirectXVertexConstantBuffer::Unbind() const
	{

	}

	std::string DirectXVertexConstantBuffer::GetUID() const noexcept
	{
		return uid;
	}

	//Pixel Constant Buffer
	DirectXPixelConstantBuffer::DirectXPixelConstantBuffer(const std::string& tag, int slot, int size, const void* data)
		:
		mSlot(slot),
		uid(tag)
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = size;
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = data;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetDevice()->CreateBuffer(&cbd, &csd, &pConstantBuffer);
	}

	void DirectXPixelConstantBuffer::SetData(int size, const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Map(
			pConstantBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0,
			&msr
		);

		memcpy(msr.pData, data, size);

		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->Unmap(pConstantBuffer.Get(), 0);
	}

	void DirectXPixelConstantBuffer::Bind() const
	{
		((DirectXRendererAPI*)RenderCommand::GetRendererAPI())->GetContext()->PSSetConstantBuffers(mSlot, 1, pConstantBuffer.GetAddressOf());
	}

	void DirectXPixelConstantBuffer::Unbind() const
	{

	}
	std::string DirectXPixelConstantBuffer::GetUID() const noexcept
	{
		return uid;
	}
}