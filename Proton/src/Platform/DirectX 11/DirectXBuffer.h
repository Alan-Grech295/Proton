#pragma once
#include "Proton\Renderer\Bindables\Buffer.h"
#include <wrl.h>
#include <d3d11.h>

#pragma comment(lib, "d3d11.lib")

namespace Proton
{
	class VertexBuffer;

	class DirectXVertexBuffer : public VertexBuffer
	{
	public:
		DirectXVertexBuffer(const std::string& tag, BufferLayout& layout, VertexShader* vertexShader);
		virtual ~DirectXVertexBuffer() {}

		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;

		virtual const BufferLayout& GetLayout() const override { return m_Layout; }

		virtual void RecreateBuffer() override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		D3D11_INPUT_ELEMENT_DESC* m_InputLayoutDesc;
	};

	class DirectXIndexBuffer : public IndexBuffer
	{
	public:
		DirectXIndexBuffer(const std::string& tag);
		virtual ~DirectXIndexBuffer() {}

		virtual void Bind() override;
	private:
		virtual void RecreateBuffer() override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_IndexBuffer;
		
	};

	class DirectXVertexConstantBuffer : public VertexConstantBuffer
	{
	public:
		DirectXVertexConstantBuffer(const std::string& tag, int slot, int size, const void* data);
		virtual ~DirectXVertexConstantBuffer() {}

		virtual void SetData(int size, const void* data) override;
		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		UINT mSlot;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
		std::string uid;
	};

	class DirectXPixelConstantBuffer : public PixelConstantBuffer
	{
	public:
		DirectXPixelConstantBuffer(const std::string& tag, int slot, int size, const void* data);
		virtual ~DirectXPixelConstantBuffer() {}

		virtual void SetData(int size, const void* data) override;
		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		UINT mSlot;
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
		std::string uid;
	};
}