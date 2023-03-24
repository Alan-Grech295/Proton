#include "ptpch.h"
#include "DynamicConstantBuffer.h"

namespace Proton::DCB
{
	LayoutElement LayoutElement::Null = {};
	struct ExtraData
	{
		struct Struct : public LayoutElement::ExtraDataBase
		{
			bool Contains(const std::string& name)
			{
				for (const auto& e : Elements)
				{
					if (e.first == name)
						return true;
				}
				return false;
			}

			std::vector<std::pair<std::string, LayoutElement>> Elements;
		};

		struct Array : public LayoutElement::ExtraDataBase
		{
			std::optional<LayoutElement> TypeElement;
			uint32_t Size;
		};
	};

	LayoutElement::LayoutElement(Type type)
		: m_Type(type)
	{
		if (type == Type::Struct)
		{
			m_ExtraData = CreateScope<ExtraData::Struct>();
		}
		else if (type == Type::Array)
		{
			m_ExtraData = CreateScope<ExtraData::Array>();
		}
	}

	void LayoutElement::Add(Type type, const std::string& name)
	{
		PT_CORE_ASSERT(m_Type == Type::Struct, "Layout element is not a struct");
		PT_CORE_ASSERT(ValidSymbolName(name), "Invalid symbol name");
		ExtraData::Struct& structData = m_ExtraData->As<ExtraData::Struct>();
		PT_CORE_ASSERT(!structData.Contains(name), "Struct already contains element with same name");

		structData.Elements.emplace_back(name, LayoutElement(type));
	}

	void LayoutElement::Set(Type type, uint32_t size)
	{
		PT_CORE_ASSERT(m_Type == Type::Array, "Layout element is not a struct");
		ExtraData::Array& arrayData = m_ExtraData->As<ExtraData::Array>();

		arrayData.TypeElement.emplace(type);
		arrayData.Size = size;
	}

	uint32_t LayoutElement::Finalize(uint32_t offsetIn)
	{
		switch (m_Type)
		{
#define X(el) case Type::el: m_Offset = AdvanceIfCrossesBoundary(offsetIn, Map<Type::el>::ShaderSize); return *m_Offset + Map<Type::el>::ShaderSize;
			LEAF_ELEMENT_TYPES
#undef X
		case Type::Struct:
			return FinalizeStruct(offsetIn);
		case Type::Array:
			return FinalizeArray(offsetIn);
		default:
			PT_CORE_ASSERT(false, "Invalid layout element");
		}
	}

	LayoutElement& LayoutElement::T()
	{
		PT_CORE_ASSERT(m_Type == Type::Array, "Layout element is not an array");
		return *m_ExtraData->As<ExtraData::Array>().TypeElement;
	}

	const uint32_t LayoutElement::GetOffsetEnd() const
	{
		switch (m_Type)
		{
#define X(el) case Type::el: return *m_Offset + Map<Type::el>::ShaderSize;
			LEAF_ELEMENT_TYPES
#undef X
		case Type::Struct:
		{
			const auto& structData = m_ExtraData->As<ExtraData::Struct>();
			return AdvanceToBoundary(structData.Elements.back().second.GetOffsetEnd());
		}
		case Type::Array:
		{
			const auto& arrayData = m_ExtraData->As<ExtraData::Array>();
			return *m_Offset + AdvanceToBoundary(arrayData.TypeElement->GetSizeBytes()) * arrayData.Size;
		}
		default:
			PT_CORE_ASSERT(false, "Empty or invalid layout element");
			return 0u;
		}
	}

	LayoutElement& LayoutElement::operator[](const std::string& name)
	{
		PT_CORE_ASSERT(m_Type == Type::Struct, "Layout element is not a struct");
		ExtraData::Struct& structData = m_ExtraData->As<ExtraData::Struct>();

		for (auto& e : structData.Elements)
		{
			if (e.first == name)
				return e.second;
		}

		return Null;
	}

	const LayoutElement& LayoutElement::operator[](const std::string& name) const
	{
		return const_cast<LayoutElement&>(*this)[name];
	}

	bool LayoutElement::ValidSymbolName(const std::string& name) const
	{
		return !name.empty() && !std::isdigit(name.front()) &&
			std::all_of(name.begin(), name.end(), [](char c) {
			return std::isalnum(c) || c == '_';
				}
		);
	}
	uint32_t LayoutElement::FinalizeStruct(uint32_t offsetIn)
	{
		auto& structData = m_ExtraData->As<ExtraData::Struct>();
		PT_CORE_ASSERT(structData.Elements.size() != 0, "Cannot finalize empty struct");

		m_Offset = AdvanceToBoundary(offsetIn);
		uint32_t nextOffset = *m_Offset;
		for (auto&[name, e] : structData.Elements)
		{
			nextOffset = e.Finalize(nextOffset);
		}
		return nextOffset;
	}

	uint32_t LayoutElement::FinalizeArray(uint32_t offsetIn)
	{
		auto& arrayData = m_ExtraData->As<ExtraData::Array>();
		PT_CORE_ASSERT(arrayData.Size > 0, "Cannot finalize empty array");
		m_Offset = AdvanceToBoundary(offsetIn);
		arrayData.TypeElement->Finalize(*m_Offset);
		return GetOffsetEnd();
	}

	Ref<LayoutElement> RawLayout::Finalize()
	{
		auto temp = std::move(m_Root);
		temp->Finalize(0);
		//Clear this layout
		*this = RawLayout();
		return temp;
	}

	ElementRef ElementRef::operator[](const std::string& name) const
	{
		PT_CORE_ASSERT(m_LayoutElement->m_Type == Type::Struct, "Element is not a struct");
		return ElementRef(&(*m_LayoutElement)[name], m_Data, 0);
		
	}
	
	ElementRef ElementRef::operator[](int index) const
	{
		PT_CORE_ASSERT(m_LayoutElement->m_Type == Type::Array, "Element is not an array");
		ExtraData::Array& arrayData = m_LayoutElement->m_ExtraData->As<ExtraData::Array>();
		return ElementRef(&*arrayData.TypeElement, m_Data, m_ArrayOffset + arrayData.TypeElement->GetSizeBytes() * index);
	}

	ElementRef Buffer::operator[](const std::string& name)
	{
		return ElementRef(&(*m_Root)[name], m_Data, 0);
	}
}