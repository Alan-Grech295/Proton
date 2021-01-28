#include "ptpch.h"
#include "DirectXBuffer.h"
#include "Platform\Windows\WindowsGraphics.h"
#include "DirectXShader.h"

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
	DirectXVertexBuffer::DirectXVertexBuffer(int stride, const void* vertices, uint32_t size)
		:
		stride(stride)
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
		WindowsGraphics::GetDevice()->CreateBuffer(&bd, &sd, &pVertexBuffer);
	}

	void DirectXVertexBuffer::Bind() const
	{
		WindowsGraphics::GetContext()->IASetVertexBuffers(0, 1u, pVertexBuffer.GetAddressOf(), &stride, 0);
		WindowsGraphics::GetContext()->IASetInputLayout(pInputLayout.Get());
	}

	void DirectXVertexBuffer::Unbind() const
	{

	}

	void DirectXVertexBuffer::SetLayout(const BufferLayout& layout, VertexShader* vertexShader)
	{
		m_Layout = layout;
		std::vector<BufferElement> elements = layout.GetElements();
		D3D11_INPUT_ELEMENT_DESC* inputLayoutDescTemp = new D3D11_INPUT_ELEMENT_DESC[elements.size()];

		for (int i = 0; i < elements.size(); i++)
		{
			inputLayoutDescTemp[i] = { elements[i].Name.c_str(), 0, ShaderDataTypeToDXGIFormat(elements[i].Type), 0, elements[i].Offset, D3D11_INPUT_PER_VERTEX_DATA, 0 };
		}

		DirectXVertexShader& dxVertexShader = *dynamic_cast<DirectXVertexShader*>(vertexShader);

		WindowsGraphics::GetDevice()->CreateInputLayout(
			inputLayoutDescTemp, elements.size(),
			dxVertexShader.pBytecodeBlob->GetBufferPointer(),
			dxVertexShader.pBytecodeBlob->GetBufferSize(),
			&pInputLayout
		);

		delete[](&elements);
		delete[](inputLayoutDescTemp);
	}

	//Index Buffer
	DirectXIndexBuffer::DirectXIndexBuffer(unsigned short* indices, uint32_t size)
		:
		count(size)
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
		WindowsGraphics::GetDevice()->CreateBuffer(&ibd, &isd, &pIndexBuffer);
	}

	void DirectXIndexBuffer::Bind() const
	{
		WindowsGraphics::GetContext()->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	}

	void DirectXIndexBuffer::Unbind() const
	{

	}

	//Vertex Constant Buffer
	DirectXVertexConstantBuffer::DirectXVertexConstantBuffer(int slot, int size, const void* data)
		:
		mSlot(slot)
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
		WindowsGraphics::GetDevice()->CreateBuffer(&cbd, &csd, &pConstantBuffer);
	}

	void DirectXVertexConstantBuffer::SetData(int size, const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		WindowsGraphics::GetContext()->Map(
			pConstantBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0,
			&msr
		);

		memcpy(msr.pData, data, size);

		WindowsGraphics::GetContext()->Unmap(pConstantBuffer.Get(), 0);
	}

	void DirectXVertexConstantBuffer::Bind() const
	{
		WindowsGraphics::GetContext()->VSSetConstantBuffers(mSlot, 1, pConstantBuffer.GetAddressOf());
	}

	void DirectXVertexConstantBuffer::Unbind() const
	{

	}

	//Pixel Constant Buffer
	DirectXPixelConstantBuffer::DirectXPixelConstantBuffer(int slot, int size, const void* data)
		:
		mSlot(slot)
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
		WindowsGraphics::GetDevice()->CreateBuffer(&cbd, &csd, &pConstantBuffer);
	}

	void DirectXPixelConstantBuffer::SetData(int size, const void* data)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		WindowsGraphics::GetContext()->Map(
			pConstantBuffer.Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0,
			&msr
		);

		memcpy(msr.pData, data, size);

		WindowsGraphics::GetContext()->Unmap(pConstantBuffer.Get(), 0);
	}

	void DirectXPixelConstantBuffer::Bind() const
	{
		WindowsGraphics::GetContext()->PSSetConstantBuffers(mSlot, 1, pConstantBuffer.GetAddressOf());
	}

	void DirectXPixelConstantBuffer::Unbind() const
	{

	}
}