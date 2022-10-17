#pragma once
#include <string>
#include <vector>
#include "Proton\Core\Log.h"
#include <unordered_map>
#include <assert.h>
#include <optional>

namespace Proton
{
#define ELEMENT_TYPES \
				X(Byte)\
				X(Int16)\
				X(Int32)\
				X(Int64)\
				X(Float)\
				X(Double)\
				X(String)\

	enum class Type {
		None = 0,
		#define X(el) el,
		ELEMENT_TYPES
		#undef X
		Struct,
		Array
	};

	namespace Meta
	{
		//For debug only
		static std::unordered_map<Type, std::string> typeToString = {
			#define X(el) {Type::el, #el},
			ELEMENT_TYPES
			#undef X
			{Type::Struct, "Struct"},
			{Type::Array, "Array"}
		};

		class Addable
		{
		public:
			virtual class Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt, uint32_t numStructChildren = 0) = 0;
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
			//Arrays are always constructed by (name, arrayType, [size])

			Element() = default;

			//Struct constructor
			//Element(const std::string& name);

			//Value Constructor
			Element(const char* name, Type type, uint32_t dataOffset, class MetaFile* metaFile = nullptr, std::string parentStructTag = "", uint32_t numStructChildren = 0);
			
			//Value Constructor
			Element(std::string name, Type type, uint32_t dataOffset, class MetaFile* metaFile = nullptr, std::string parentStructTag = "", uint32_t numStructChildren = 0);


			//Array constructor
			//TODO: Confirm that given type has constant size
			//Element(const std::string& name, Type arrayType, uint32_t size, bool constElementSize = true);

			//Debug only
			std::string ToString();

			static Element& Empty()
			{
				static Element empty = { "", Type::None, 0 };
				return empty;
			}

			void ToBytes(std::vector<byte>& bytes);

			//Struct Only
			virtual Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt, uint32_t numStructChildren = 0) override;

			Element& operator[](const std::string& name);

			//Array only
			Element& GetType();

			Element& SetSize(uint32_t size);

			Element& SetType(Type type);

			Element& SetElementOffsets(std::vector<uint32_t> offsets);

		private:
			//The data in bytes when stored in an array element
			uint32_t CalculateSizeInArray();

			std::pair<byte*, uint32_t> GetDataForArray();

			//Struct Only
			//void Add(Element element);
		public:
			const char* m_Name;
			Type m_Type;
			ExtraDataBase* m_ExtraData;
			//The byte offset to access the data
			uint32_t m_DataOffset;

		};

		class MetaFile : public Addable
		{
			friend class MetaFileSerializer;
			friend class Element;
		public:
			MetaFile(uint32_t numElements)
			{
				m_Elements.reserve(numElements);
			}

			/*void Add(Element element)
			{
				m_Elements.push_back(std::move(element));
				m_ElementLocator[m_Elements.back().m_Name] = &m_Elements.back();
			}*/

			virtual Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt, uint32_t numStructChildren = 0) override
			{
				assert("No data offset was given!" && dataOffset.has_value());
				auto [it, succeded] = m_ElementLocator.emplace(name, (m_Elements.data() + m_Elements.size()));
				m_Elements.push_back(Element(it->first.c_str(), type, dataOffset.value(), this, "", numStructChildren));
				return m_Elements.back();
			}

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

			//private:
			//TODO: Set to private function
			std::string GetNextStructTag()
			{
				std::string out;
				if (m_NextStructTag <= 0xF)
				{
					out = std::string((const char*)(&m_NextStructTag), 1);
				}
				else if (m_NextStructTag <= 0xFF)
				{
					out = std::string((const char*)(&m_NextStructTag), 2);
				}
				else if (m_NextStructTag <= 0xFFF)
				{
					out = std::string((const char*)(&m_NextStructTag), 3);
				}
				else
				{
					out = std::string((const char*)(&m_NextStructTag), 4);
				}
				m_NextStructTag++;
				if (m_NextStructTag == (int)'|')
					m_NextStructTag++;
				return out;
			}

			void AddStructElement(const std::string& name, Type type, uint32_t dataOffset, uint32_t numStructChildren, const std::string& structTag, std::vector<Element>& elements)
			{
				auto [it, succeded] = m_ElementLocator.emplace(structTag + name, (Element*)(elements.data() + elements.size()));
				elements.push_back(Element(it->first.c_str() + structTag.length(), type, dataOffset, this, structTag, numStructChildren));
			}

		private:
			std::vector<Element> m_Elements;
			std::unordered_map<std::string, Element*> m_ElementLocator;
			
			//Next struct tag
			uint32_t m_NextStructTag = 1;
		};

		class MetaFileSerializer
		{
		public:
			MetaFileSerializer()
			{

			}

			static MetaFile& DeserializeMetaFile(const std::string& path);

			static void SerializeMetaFile(const std::string& path, MetaFile& file);

		private:
			static Element ReadElement(byte*& data, MetaFile& file, Element::ExtraDataBase* structData = nullptr);

			//TODO: Support non-const size arrays
			static Element& ReadArrayElement(byte*& data);
		};

		template<typename T>
		inline T& Element::ExtraDataBase::as()
		{
			return static_cast<T&>(*this);
		}

#define COPY_DATA(dest, src, size) memcpy(dest, src, size); ptr += size
}
}