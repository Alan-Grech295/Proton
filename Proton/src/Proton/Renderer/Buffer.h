#pragma once
#include <string>
#include <cassert>
#include "Shader.h"
#include "Proton\Core\Log.h"

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

		BufferElement() 
			:
			Name("UNKNOWN"),
			Type(ShaderDataType::None),
			Size(0),
			Offset(0)
		{}

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

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<VertexBuffer> Create(const std::string& tag, int stride, const void* vertices, uint32_t size);
		
		static Scope<VertexBuffer> CreateUnique(int stride, const void* vertices, uint32_t size);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual unsigned int GetCount() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<IndexBuffer> Create(const std::string& tag, unsigned short* indices, uint32_t size);
		
		static Scope<IndexBuffer> CreateUnique(unsigned short* indices, uint32_t size);
	};

	class VertexConstantBuffer
	{
	public:
		virtual ~VertexConstantBuffer() {}

		virtual void SetData(int size, const void* data) = 0;
		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<VertexConstantBuffer> Create(const std::string& tag, int slot, int size, const void* data);
		
		static Scope<VertexConstantBuffer> CreateUnique(int slot, int size, const void* data);
	};

	class PixelConstantBuffer
	{
	public:
		virtual ~PixelConstantBuffer() {}

		virtual void SetData(int size, const void* data) = 0;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<PixelConstantBuffer> Create(const std::string& tag, int slot, int size, const void* data);

		static Scope<PixelConstantBuffer> CreateUnique(int slot, int size, const void* data);
	};
}