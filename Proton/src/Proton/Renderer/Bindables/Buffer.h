#pragma once
#include <string>
#include <cassert>
#include "Shader.h"
#include "Bindable.h"
#include "DynamicConstantBuffer.h"

#include <initializer_list>

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

		assert("Unknown Shader Data Type!");
		return 0;
	}

	struct BufferElement
	{
		BufferElement()
			:
			Name("UNKNOWN"),
			Type(ShaderDataType::None),
			Size(0),
			Offset(0)
		{}

		BufferElement(const std::string& name, ShaderDataType type)
			:
			Name(name),
			Type(type),
			Size(ShaderDataTypeSize(type)),
			Offset(0)
		{
		}

		std::string Name;
		ShaderDataType Type;
		uint32_t Offset;
		uint32_t Size;
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

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }

		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }

		uint32_t size() { return (uint32_t)m_Elements.size(); }
		uint32_t size() const { return (uint32_t)m_Elements.size(); } const

		BufferElement& operator[](int index) 
		{
			return m_Elements[index];
		}

		const BufferElement& operator[](int index) const
		{
			return m_Elements[index];
		}

		BufferElement& operator[](const std::string& tag)
		{
			//return *m_ElementByTag[tag];
			for (BufferElement& e : m_Elements)
			{
				if (e.Name == tag)
					return e;
			}

			return BufferElement();
		}
	private:
		void CalculateOffsetsAndStride()
		{
			uint32_t offset = 0;

			for (BufferElement& element : m_Elements)
			{
				//m_ElementByTag[element.Name] = &element;
				element.Offset = offset;
				offset += element.Size;
			}

			stride = offset;
		}
	public:
		uint32_t stride = 0;
	private:
		std::vector<BufferElement> m_Elements;
		//std::unordered_map<std::string, BufferElement*> m_ElementByTag;
	};

	class Vertex
	{
	public:
		Vertex (char* data, BufferLayout& bufferLayout, bool* bufferChanged = nullptr)
			:
			m_Data(data),
			m_BufferLayout(bufferLayout),
			m_BufferChanged(bufferChanged)
		{}

		//TODO: Get a math library
		template<typename T>
		const auto& Get(const std::string& tag) const
		{
			BufferElement& element = m_BufferLayout[tag];
			char* data = m_Data + element.Offset;
			assert((ShaderDataTypeSize(element.Type) == sizeof(T)) && "The specified type is not compatible with the element size!");

			return *reinterpret_cast<T*>(data);
		}

		template<typename T>
		auto& Get(const std::string& tag)
		{
			BufferElement& element = m_BufferLayout[tag];
			char* data = m_Data + element.Offset;
			assert((ShaderDataTypeSize(element.Type) == sizeof(T)) && "The specified type is not compatible with the element size!");

			if (m_BufferChanged)
				*m_BufferChanged = true;

			return *reinterpret_cast<T*>(data);
		}

		template<typename ...Params>
		void Set(Params&&... params)
		{
			SetAttributeByIndex(0, std::forward<Params>(params)...);
			if (m_BufferChanged)
				*m_BufferChanged = true;
		}

	private:
		template<typename T>
		void SetAttributeByIndex(int index, T&& val)
		{
			auto& element = m_BufferLayout[index];
			assert("The specified value has a different size from the destination size" && element.Size == sizeof(T));

			char* dataPos = m_Data + element.Offset;
			*reinterpret_cast<std::remove_reference<T>::type*>(dataPos) = val;
		}

		template<typename First, typename ...Rest>
		void SetAttributeByIndex(int index, First&& first, Rest&&... rest)
		{
			SetAttributeByIndex(index, std::forward<First>(first));
			SetAttributeByIndex(index + 1, std::forward<Rest>(rest)...);
		}

	private:
		char* m_Data;
		BufferLayout& m_BufferLayout;
		bool* m_BufferChanged = nullptr;
	};

	class VertexBuffer : public Bindable
	{
		friend class BindsCollection;
	public:
		virtual ~VertexBuffer() {}

		virtual const BufferLayout& GetLayout() const = 0;

		static Ref<VertexBuffer> Create(const std::string& tag, BufferLayout& layout, VertexShader* vertexShader, uint32_t numElements);
		
		static Scope<VertexBuffer> CreateUnique(BufferLayout& layout, VertexShader* vertexShader, uint32_t numElements);

		static Scope<VertexBuffer> CreateUnique(Ref<Bindable> other);
	
		//Vertex Buffer Interface Functions
		template<typename ...Params>
		Vertex& EmplaceBack(Params&&... params)
		{
			Vertex vert = Vertex{ m_End, m_Layout, &m_Changed };
			vert.Set(std::forward<Params>(params)...);

			m_End += m_Layout.stride;

			m_Changed = true;

			return vert;
		}

		Vertex& operator[](int index)
		{
			return Vertex{ m_Data + index * m_Layout.stride, m_Layout, &m_Changed };
		}

		void SetRawData(void* rawData, uint32_t size)
		{
			if (m_MaxSize != size)
			{
				delete[] m_Data;
				m_Data = (char*)malloc(size);
			}
			memcpy(m_Data, rawData, size);
			m_End = m_Data + size;
			m_Changed = true;
		}

		void Clear()
		{
			delete[] m_Data;
			m_Data = nullptr;
			m_End = nullptr;
			m_Changed = true;
		}

		uint32_t size() { return (uint32_t)(m_End - m_Data) / m_Layout.stride; }

		uint32_t capacity() { return m_MaxSize; }

		uint32_t sizeBytes() { return (uint32_t)(m_End - m_Data); }

		//IMP: Go back to protected (public for testing)
	//protected:
	public:
		virtual void Bind() = 0;

	protected:
		virtual void RecreateBuffer() = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		virtual std::string GetUID() const noexcept { return m_Uid; }

	protected:
		std::string m_Uid;
		VertexShader* m_Shader;
		char* m_Data;
		char* m_End;
		uint32_t m_MaxSize = 0;
		BufferLayout m_Layout;
		bool m_Changed = true;

		int m_PastBufferSize = -1;
	};

	class IndexBuffer : public Bindable
	{
		friend class BindsCollection;
	public:
		virtual ~IndexBuffer() {}

		virtual void Bind() = 0;

		uint32_t size() { return (uint32_t)m_Indices.size(); }

		void EmplaceBack(uint32_t index)
		{
			m_Indices.push_back(index);
			m_Changed = true;
		}

		void SetRawData(uint32_t* data, uint32_t size)
		{
			m_Indices.assign(data, data + size);
			m_Changed = true;
		}

		uint32_t& operator[](int index)
		{
			m_Changed = true;
			return m_Indices[index];
		}

		const uint32_t& operator[](int index) const
		{
			return m_Indices[index];
		}

		static Ref<IndexBuffer> Create(const std::string& tag, uint32_t size);
		
		static Scope<IndexBuffer> CreateUnique(uint32_t size);

		static Scope<IndexBuffer> CreateUnique(Ref<Bindable> other);

	protected:
		virtual void RecreateBuffer() = 0;

		std::string GetUID() const noexcept { return m_Uid; }

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

	protected:
		std::vector<uint32_t> m_Indices;
		std::string m_Uid;
		bool m_Changed = true;

		int m_PastBufferSize = -1;
	};

	class VertexConstantBuffer : public Bindable
	{
	public:
		virtual ~VertexConstantBuffer() {}

		virtual void SetData(const void* data, int size = -1) = 0;
		virtual void Bind() = 0;
		virtual void* GetData() = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<VertexConstantBuffer> Create(const std::string& tag, int slot, int size, const void* data);
		
		static Scope<VertexConstantBuffer> CreateUnique(int slot, int size, const void* data);

		static Scope<VertexConstantBuffer> CreateUnique(Ref<Bindable> other);

	protected:
		uint32_t m_Size;
		uint32_t m_Slot;
	};

	class PixelConstantBuffer : public Bindable
	{
	public:
		virtual ~PixelConstantBuffer() {}

		virtual void SetData(const void* data, int size = -1) = 0;
		virtual void Bind() = 0;
		virtual void* GetData() = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<PixelConstantBuffer> Create(const std::string& tag, int slot, int size, const void* data);

		static Scope<PixelConstantBuffer> CreateUnique(int slot, int size, const void* data);

		static Scope<PixelConstantBuffer> CreateUnique(Ref<Bindable> other);

		static PixelConstantBuffer* CreateUniquePtr(int slot, int size, const void* data);

	protected:
		uint32_t m_Size;
		uint32_t m_Slot;
	};
}