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
			//Arrays are always constructed by (name, arrayType, [size])

			Element() = default;

			//Struct constructor
			//Element(const std::string& name);

			//Value Constructor
			Element(const char* name, Type type, uint32_t dataOffset);
			
			//Value Constructor
			Element(std::string name, Type type, uint32_t dataOffset);


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
			uint32_t m_SizeBytes;
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

			virtual Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset = std::nullopt) override
			{
				assert("No data offset was given!" && dataOffset.has_value());
				auto [it, succeded] = m_ElementLocator.emplace(name, (m_Elements.data() + m_Elements.size()));
				m_Elements.push_back(Element(it->first.c_str(), type, dataOffset.value()));
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

		private:
			std::vector<Element> m_Elements;
			std::unordered_map<std::string, Element*> m_ElementLocator;
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
			static Element ReadElement(byte*& data, Addable& addable);

			//TODO: Support non-const size arrays
			//readName - Reads the names of variables in a struct
			static Element ReadArrayElement(byte*& data, bool readName = false);
		};

		template<typename T>
		inline T& Element::ExtraDataBase::as()
		{
			return static_cast<T&>(*this);
		}

//#define COPY_DATA(dest, src, size) memcpy(dest, src, size); ptr += size
#define COPY_DATA_OFFSET(start, offset, src, size) memcpy((start + offset), src, size); offset += size

}
}