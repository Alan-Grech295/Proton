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
		virtual ~DirectXVertexBuffer() 
		{
			delete[] m_InputLayoutDesc;
		}

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
		virtual ~DirectXVertexConstantBuffer()
		{
			delete[] m_Data;
		}

		virtual void SetData(const void* data, int size = -1) override;
		virtual void* GetData() override;
		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
		uint8_t* m_Data;
		std::string uid;
	};

	class DirectXPixelConstantBuffer : public PixelConstantBuffer
	{
	public:
		DirectXPixelConstantBuffer(const std::string& tag, int slot, DCB::CookedLayout& layout);
		virtual ~DirectXPixelConstantBuffer()
		{
			delete[] m_Data;
		}

		virtual uint8_t* GetData() override;
		virtual void Bind() override;

		virtual std::string GetUID() const noexcept override;
	private:
		void SetData();
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
		std::string uid;
	};
}