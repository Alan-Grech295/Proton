#pragma once
#include "Proton/Core/Core.h"

#include <optional>
#include <DirectXMath.h>

namespace Proton
{
#define LEAF_ELEMENT_TYPES \
						X(Float)\
						X(Float2)\
						X(Float3)\
						X(Float4)\
						X(Matrix4x4)\
						X(Int)\
						X(Bool)

	enum class ElementType
	{
		None = 0,
#define X(el) el,
		LEAF_ELEMENT_TYPES
#undef X
		Struct,
		Array
	};

	//TODO: Generalize to different shaders
	template<ElementType Type>
	struct Map
	{
		static constexpr bool valid = false;
	};

	template<> struct Map<ElementType::Float>
	{
		using SysType = float;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F1";

		static constexpr bool valid = true;
	};

	template<> struct Map<ElementType::Float2>
	{
		using SysType = DirectX::XMFLOAT2;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F2";

		static constexpr bool valid = true;
	};

	template<> struct Map<ElementType::Float3>
	{
		using SysType = DirectX::XMFLOAT3;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F3";

		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Float4>
	{
		using SysType = DirectX::XMFLOAT4;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "F4";

		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Matrix4x4>
	{
		using SysType = DirectX::XMFLOAT4X4;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "M4x4";

		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Int>
	{
		using SysType = int;
		static constexpr uint32_t ShaderSize = sizeof(SysType);
		static constexpr char* code = "I1";

		static constexpr bool valid = true;
	};
	template<> struct Map<ElementType::Bool>
	{
		using SysType = bool;
		static constexpr uint32_t ShaderSize = sizeof(BOOL);
		static constexpr char* code = "B";

		static constexpr bool valid = true;
	};

#define X(el) static_assert(Map<ElementType::el>::valid, "Missing map implementation for "#el);
	LEAF_ELEMENT_TYPES
#undef X

	template<typename T>
	struct ReverseMap
	{
		static constexpr bool valid = false;
	};

#define X(el)\
	template<> struct ReverseMap<Map<ElementType::el>::SysType> \
	{ \
		static constexpr ElementType type = ElementType::el; \
		static constexpr bool valid = true; \
	};
	LEAF_ELEMENT_TYPES
#undef X

	class LayoutElement
	{
	public:
		struct ExtraDataBase
		{
			virtual ~ExtraDataBase();

			template<typename T>
			T& As() { return *static_cast<T*>(this); }
		};

		LayoutElement() = default;
		LayoutElement(ElementType type);

		template<ElementType Type>
		void Add(const std::string& name);

		template<ElementType Type>
		void Set(uint32_t size);

		uint32_t GetOffset() { return *m_Offset; }
		uint32_t GetOffsetEnd() { return *m_Offset + GetSizeBytes(); };
		uint32_t GetSizeBytes();

		LayoutElement& operator[](const std::string& name);
		const LayoutElement& operator[](const std::string& name) const;
		operator bool() { return m_Type != ElementType::None; }
	private:
		bool ValidSymbolName(const std::string& name);

		uint32_t AdvanceToBoundary(uint32_t offset) { return offset + (ALIGNMENT - offset % ALIGNMENT) % ALIGNMENT; }
	
		bool CrossesBoundary(uint32_t offset, uint32_t size)
		{
			if (size > ALIGNMENT) return true;
			const uint32_t end = offset + size;
			const uint32_t pageStart = offset / 16u;
			const uint32_t pageEnd = end / 16u;
			return pageStart != pageEnd && end % 16 != 0u;
		}

		uint32_t AdvanceIfCrossesBoundary(uint32_t offset, uint32_t size) { return CrossesBoundary(offset, size) ? AdvanceToBoundary(offset) : offset; }
	public:
		ElementType m_Type = ElementType::None;
		std::optional<uint32_t> m_Offset;
		Scope<ExtraDataBase> m_ExtraData;

		static LayoutElement Null;

		//TODO: Set depending on graphics API
		inline static uint32_t ALIGNMENT = 16u;
	};
}