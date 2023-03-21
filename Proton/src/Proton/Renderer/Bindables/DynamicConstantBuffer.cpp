#include "ptpch.h"
#include "DynamicConstantBuffer.h"

namespace Proton
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

	LayoutElement::LayoutElement(ElementType type)
		: m_Type(type)
	{
		if (type == ElementType::Struct)
		{
			m_ExtraData = CreateScope<ExtraData::Struct>();
		}
		else if (type == ElementType::Array)
		{
			m_ExtraData = CreateScope<ExtraData::Array>();
		}
	}

	template<ElementType Type>
	void LayoutElement::Add(const std::string& name)
	{
		PT_CORE_ASSERT(m_Type == ElementType::Struct, "Element type is not a struct");
		PT_CORE_ASSERT(ValidSymbolName(name), "Invalid symbol name");
		ExtraData::Struct& structData = m_ExtraData->As<ExtraData::Struct>();
		PT_CORE_ASSERT(!structData.Contains(name), "Struct already contains element with same name");

		structData.Elements.emplace_back(Type, name);
	}

	template<ElementType Type>
	void LayoutElement::Set(uint32_t size)
	{
		PT_CORE_ASSERT(m_Type == ElementType::Array, "Element type is not a struct");
		ExtraData::Array& arrayData = m_ExtraData->As<ExtraData::Array>();

		arrayData.TypeElement.emplace(Type);
		arrayData.Size = size;
	}

	uint32_t LayoutElement::GetSizeBytes()
	{
		switch (m_Type)
		{
#define X(el) case ElementType::el: return Map<ElementType::el>::ShaderSize;
			LEAF_ELEMENT_TYPES
#undef X
		case ElementType::Struct:
			const auto& structData = m_ExtraData->As<ExtraData::Struct>();

			return 0;
		case ElementType::Array:

			return 0;
		}
	}

	LayoutElement& LayoutElement::operator[](const std::string& name)
	{
		PT_CORE_ASSERT(m_Type == ElementType::Struct, "Element type is not a struct");
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

	bool LayoutElement::ValidSymbolName(const std::string& name)
	{
		return !name.empty() && !std::isdigit(name.front()) &&
			std::all_of(name.begin(), name.end(), [](char c) {
			return std::isalnum(c) || c == '_';
				}
		);
	}
}