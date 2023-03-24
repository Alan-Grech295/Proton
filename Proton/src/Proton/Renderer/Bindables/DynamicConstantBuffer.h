#pragma once
#include "Proton/Core/Core.h"

#include <optional>
#include <DirectXMath.h>

namespace Proton::DCB
{
#define LEAF_ELEMENT_TYPES \
						X(Float)\
						X(Float2)\
						X(Float3)\
						X(Float4)\
						X(Matrix4x4)\
						X(Int)\
						X(Bool)

	enum class Type
	{
		None = 0,
#define X(el) el,
		LEAF_ELEMENT_TYPES
#undef X
		Struct,
		Array
	};

	//TODO: Generalize to different shaders
	template<Type Type>
	struct Map
	{
		static constexpr bool valid = false;
	};

	template<> struct Map<Type::Float>
	{
		using SysType = float;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F1";

		static constexpr bool valid = true;
	};

	template<> struct Map<Type::Float2>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F2";

		static constexpr bool valid = true;
	};

	template<> struct Map<Type::Float3>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F3";

		static constexpr bool valid = true;
	};
	template<> struct Map<Type::Float4>
	{
		using SysType = DirectX::XMFLOAT4;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F4";

		static constexpr bool valid = true;
	};
	template<> struct Map<Type::Matrix4x4>
	{
		using SysType = DirectX::XMFLOAT4X4;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "M4x4";

		static constexpr bool valid = true;
	};
	template<> struct Map<Type::Int>
	{
		using SysType = int;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "I1";

		static constexpr bool valid = true;
	};
	template<> struct Map<Type::Bool>
	{
		using SysType = bool;
		static constexpr uint32_t ShaderSize = sizeof(BOOL);
		static constexpr char* code = "B";

		static constexpr bool valid = true;
	};

#define X(el) static_assert(Map<Type::el>::valid, "Missing Map implementation for "#el);
	LEAF_ELEMENT_TYPES
#undef X

	template<typename T>
	struct ReverseMap
	{
		static constexpr bool valid = false;
	};

#define X(el)\
	template<> struct ReverseMap<Map<Type::el>::SysType> \
	{ \
		static constexpr Type type = Type::el; \
		static constexpr bool valid = true; \
	};
	LEAF_ELEMENT_TYPES
#undef X

	class LayoutElement
	{
	public:
		struct ExtraDataBase
		{
			virtual ~ExtraDataBase() = default;

			template<typename T>
			T& As() { return *static_cast<T*>(this); }
		};

		LayoutElement() = default;
		LayoutElement(Type type);

		void Add(Type type, const std::string& name);

		void Set(Type type, uint32_t size);

		bool Exists() const { return m_Type != Type::None; }

		uint32_t Finalize(uint32_t offsetIn);

		LayoutElement& T();

		const uint32_t GetOffset() const { return *m_Offset; }
		const uint32_t GetOffsetEnd() const;
		const uint32_t GetSizeBytes() const { return GetOffsetEnd() - GetOffset(); }

		LayoutElement& operator[](const std::string& name);
		const LayoutElement& operator[](const std::string& name) const;
	private:
		bool ValidSymbolName(const std::string& name) const;

		//TODO: Move to API dependent functions
		uint32_t AdvanceToBoundary(uint32_t offset) const { return offset + (ALIGNMENT - offset % ALIGNMENT) % ALIGNMENT; }
	
		bool CrossesBoundary(uint32_t offset, uint32_t size) const
		{
			if (size > ALIGNMENT) return true;
			const uint32_t end = offset + size;
			const uint32_t pageStart = offset / 16u;
			const uint32_t pageEnd = end / 16u;
			return pageStart != pageEnd && end % 16 != 0u;
		}

		uint32_t AdvanceIfCrossesBoundary(uint32_t offset, uint32_t size) const { return CrossesBoundary(offset, size) ? AdvanceToBoundary(offset) : offset; }
	
		uint32_t FinalizeStruct(uint32_t offsetIn);
		uint32_t FinalizeArray(uint32_t offsetIn);
	public:
		Type m_Type = Type::None;
		std::optional<uint32_t> m_Offset;
		Scope<ExtraDataBase> m_ExtraData;

		static LayoutElement Null;

		//TODO: Set depending on graphics API
		inline static uint32_t ALIGNMENT = 16u;
	};

	class Layout
	{
	public:
		uint32_t GetSizeInBytes() const { return m_Root->GetSizeBytes(); }
	protected:
		Layout(Ref<LayoutElement> root)
			: m_Root(std::move(root)) {}

		Ref<LayoutElement> m_Root;
	};

	class RawLayout : public Layout
	{
	public:
		RawLayout()
			: Layout(CreateRef<LayoutElement>(Type::Struct))
		{}

		LayoutElement& operator[](const std::string& name) { return (*m_Root)[name]; }

		void Add(Type type, const std::string& name) { (*m_Root).Add(type, name); }

		Ref<LayoutElement> Finalize();
	};
	
	class CookedLayout : public Layout
	{
	public:
		CookedLayout(RawLayout&& layout)
			: Layout(std::move(layout.Finalize()))
		{
		}

		const LayoutElement& operator[](const std::string& name) { return (*m_Root)[name]; }
	
		uint32_t Size() const { return m_Root->GetSizeBytes(); }
	};

	class ElementRef
	{
		friend class Buffer;
	public:
		ElementRef operator[](const std::string& name) const;
		ElementRef operator[](int index) const;

		template<typename T>
		explicit operator T&() const
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in conversion");
			return *reinterpret_cast<T*>(m_Data + m_ArrayOffset + m_LayoutElement->GetOffset());
		}

		// assignment for writing to as a supported SysType
		template<typename T>
		T& operator=(const T& rhs) const
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in assignment");
			return static_cast<T&>(*this) = rhs;
		}

	private:
		ElementRef(const LayoutElement* element, uint8_t* data, uint32_t arrayOffset)
			: m_LayoutElement(element), m_Data(data), m_ArrayOffset(arrayOffset)
		{
		}

		const LayoutElement* m_LayoutElement;
		uint8_t* m_Data;
		uint32_t m_ArrayOffset;
	};

	class Buffer
	{
	public:
		Buffer(RawLayout&& layout)
			: m_Root(std::move(layout.Finalize()))
		{
			m_Data = new uint8_t[m_Root->GetSizeBytes()];
		}

		~Buffer()
		{
			delete[] m_Data;
		}

		ElementRef operator[](const std::string& name);

	private:
		Ref<LayoutElement> m_Root;
		uint8_t* m_Data;
	};
}