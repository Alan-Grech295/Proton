#pragma once
#include "Proton\Renderer\Buffer.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class VertexBuffer;

	class DirectXVertexBuffer : public VertexBuffer
	{
	public:
		DirectXVertexBuffer(int stride, const void* vertices, uint32_t size);
		virtual ~DirectXVertexBuffer() {}

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual const BufferLayout GetLayout() const override { return m_Layout; }
		virtual void SetLayout(const BufferLayout& layout, VertexShader* vertexShader) override;
	private:
		UINT stride;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
		BufferLayout m_Layout;
	};

	class DirectXIndexBuffer : public IndexBuffer
	{
	public:
		DirectXIndexBuffer(unsigned short* indices, uint32_t size);
		virtual ~DirectXIndexBuffer() {}

		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		UINT count;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	};

	class DirectXVertexConstantBuffer : public VertexConstantBuffer
	{
	public:
		DirectXVertexConstantBuffer(int slot, int size, const void* data);
		virtual ~DirectXVertexConstantBuffer() {}

		virtual void SetData(int size, const void* data) override;
		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		UINT mSlot;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	};

	class DirectXPixelConstantBuffer : public PixelConstantBuffer
	{
	public:
		DirectXPixelConstantBuffer(int slot, int size, const void* data);
		virtual ~DirectXPixelConstantBuffer() {}

		virtual void SetData(int size, const void* data) override;
		virtual void Bind() const override;
		virtual void Unbind() const override;
	private:
		UINT mSlot;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	};
}