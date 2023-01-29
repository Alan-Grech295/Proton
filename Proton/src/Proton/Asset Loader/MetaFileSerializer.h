#pragma once
#include <string>
#include <vector>
#include "Proton\Core\Log.h"
#include <unordered_map>
#include <assert.h>
#include <optional>
#include "TypeMap.h"

namespace Proton
{
	namespace Meta
	{
		//For debug only
		static std::unordered_map<Type, std::string> typeToString = {
			#define X(el) {Type::el, #el},
			ELEMENT_TYPES
			#undef X
			{Type::Struct, "Struct"},
			{Type::Array, "Array"},
			{Type::Pointer, "Pointer"}
		};

		//Virtual class for Element and MetaFile so that elements can be added to them
		class Addable
		{
		public:
			virtual class Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt) = 0;
		};

		class Element : public Addable
		{
			friend class MetaFileSerializer;
			struct ExtraDataBase
			{
				virtual ~ExtraDataBase() = default;

				template<typename T>
				T& as();
			};

			friend struct ExtraData;

		public:
			Element() = default;

			Element(const char* name, Type type, uint32_t dataOffset);
			
			Element(std::string name, Type type, uint32_t dataOffset);

			//Copy contructor
			Element(const Element& element);

			//Debug only
			std::string ToString();

			//Empty Element
			static Element& Empty()
			{
				static Element empty = { "", Type::None, 0 };
				return empty;
			}

			void ToBytes(std::vector<byte>& bytes);

			//Struct Only
			virtual Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt) override;

			Element& operator[](const std::string& name);

			//Array only
			Element& GetType();

			Element& SetSize(uint32_t size);

			Element& SetType(Type type);

			Element& SetElementOffsets(std::vector<uint32_t> offsets);

			//Size of the data in bytes
			uint32_t GetSizeInBytes()
			{
				assert("Size is 0" && m_SizeBytes > 0);
				return m_SizeBytes;
			}

		private:
			//The data in bytes when stored in an array element
			uint32_t CalculateSizeInArray();
			//Size of bytes when saved
			uint32_t CalculateSizeBytes();

			std::vector<byte> GetDataForArray();

			//Struct Only
			//void Add(Element element);
		public:
			const char* m_Name;
			Type m_Type;
			ExtraDataBase* m_ExtraData;
			//The byte offset to access the data
			uint32_t m_DataOffset;

			//IMP: HAVE TO SET IT DIRECTLY (cannot through constructor)
			uint32_t m_SizeBytes = 0;
		};

		class MetaFile : public Addable
		{
			friend class MetaFileSerializer;
			friend class Element;
		public:
			MetaFile(uint32_t numElements)
			{
				//Elements are reserved so that the vector is not reallocated
				//so m_ElementLocator can access the elements by pointer
				m_Elements.reserve(numElements);
			}

			virtual Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt) override
			{
				assert("No data offset was given!" && dataOffset.has_value());
				//Emplacing the pointer in the element locator before creating the element so that
				//the same name char* of the element locator key can be used as the name of the element
				auto [it, succeded] = m_ElementLocator.emplace(name, (m_Elements.data() + m_Elements.size()));
				m_Elements.push_back(Element(it->first.c_str(), type, dataOffset.value()));
				return m_Elements.back();
			}

			//For debug only
			std::string ToString()
			{
				std::string str;

				for (Element& e : m_Elements)
				{
					str += e.ToString() + "\n";
				}

				return str;
			}

			Element& operator[](const std::string& name)
			{
				return *m_ElementLocator[name];
			}

		private:
			std::vector<Element> m_Elements;
			std::unordered_map<std::string, Element*> m_ElementLocator;
		};

		class MetaFileSerializer
		{
		public:
			static MetaFile& DeserializeMetaFile(const std::string& path);

			static void SerializeMetaFile(const std::string& path, MetaFile& file);

		private:
			static Element ReadElement(byte*& data, Addable& addable);

			//readName - Reads the names of variables in a struct (only when struct is a type template)
			static Element ReadArrayElement(byte*& data, bool readName = false);
		};

		template<typename T>
		inline T& Element::ExtraDataBase::as()
		{
			return static_cast<T&>(*this);
		}

	//Copies data to a memory location based on a base value and offset, then increases the offset
	#define COPY_DATA_OFFSET(start, offset, src, size) memcpy((start + offset), src, size); offset += size
	}
}