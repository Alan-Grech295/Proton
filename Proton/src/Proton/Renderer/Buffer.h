#pragma once
#include <string>
#include <cassert>
#include "Shader.h"
#include <cassert>

namespace Proton
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Matrix3x3, Matrix4x4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
			case ShaderDataType::Float:			return 4;
			case ShaderDataType::Float2:		return 4 * 2;
			case ShaderDataType::Float3:		return 4 * 3;
			case ShaderDataType::Float4:		return 4 * 4;
			case ShaderDataType::Matrix3x3:		return 4 * 3 * 3;
			case ShaderDataType::Matrix4x4:		return 4 * 4 * 4;
			case ShaderDataType::Int:			return 4;
			case ShaderDataType::Int2:			return 4 * 2;
			case ShaderDataType::Int3:			return 4 * 3;
			case ShaderDataType::Int4:			return 4 * 4;
			case ShaderDataType::Bool:			return 1;
		}

		assert("Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32_t Offset;
		uint32_t Size;

		BufferElement() {}

		BufferElement(const std::string name, ShaderDataType type)
			:
			Name(name),
			Type(type),
			Size(ShaderDataTypeSize(type)),
			Offset(0)
		{

		}
	};

	class BufferLayout
	{
	public:
		BufferLayout() {}

		BufferLayout(const std::initializer_list<BufferElement>& elements) 
			:
			m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline const uint32_t GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;
			m_Stride = 0;

			for (auto& element : m_Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				m_Stride += element.Size;
			}
		}
	private:
		std::vector<BufferElement> m_Elements;
		uint32_t m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual const BufferLayout GetLayout() const = 0;
		virtual void SetLayout(const BufferLayout& layout, VertexShader* vertexShader) = 0;
		
		static VertexBuffer* Create(int stride, const void* vertices, uint32_t size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual unsigned int GetCount() const = 0;

		static IndexBuffer* Create(unsigned short* indices, uint32_t size);
	};

	class VertexConstantBuffer
	{
	public:
		virtual ~VertexConstantBuffer() {}

		virtual void SetData(int size, const void* data) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static VertexConstantBuffer* Create(int slot, int size, const void* data);
	};

	class PixelConstantBuffer
	{
	public:
		virtual ~PixelConstantBuffer() {}

		virtual void SetData(int size, const void* data) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static PixelConstantBuffer* Create(int slot, int size, const void* data);
		static PixelConstantBuffer* Create(int slot = 0);
	};
}