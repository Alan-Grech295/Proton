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

			bool Has(const std::string& name) const;

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

		//Extra data for elements
		struct ExtraData
		{
			//Struct extra data
			struct Struct : public Element::ExtraDataBase
			{
				Struct() = default;

				Struct(Element::ExtraDataBase* other)
				{
					Struct& structData = other->as<Struct>();
					m_Elements.resize(structData.m_Elements.size());
					memcpy(m_Elements.data(), structData.m_Elements.data(), sizeof(Element) * structData.m_Elements.size());
				}

				Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset)
				{
					//If data offset has no value then it is assumed that the element is used as an array type template
					m_Elements.push_back(Element(name, type, dataOffset.value_or(0)));

					return m_Elements.back();
				}

				Element& operator[](const std::string& name)
				{
					/*auto [cacheName, cacheIndex] = m_CachedElement;
					if (cacheName == name)
						return m_Elements[cacheIndex];*/

					for (Element& e : m_Elements)
					{
						if (e.m_Name == name)
							return e;
					}

					assert("Element not found" && false);
					return Element::Empty();
				}

				bool Has(const std::string& name)
				{
					int index = 0;
					for (Element& e : m_Elements)
					{
						if (e.m_Name == name)
						{
							//m_CachedElement = std::make_tuple(name, index);
							return true;
						}

						index++;
					}

					return false;
				}

				//Calculates the size of the struct if it were constant size
				int CalcStructSize(int offset)
				{
					int size = 0;
					for (Element& element : m_Elements)
					{
						element.m_DataOffset = size + offset;
						assert("Struct not of constant size" && !(element.m_Type == Type::String || element.m_Type == Type::Array));
						if (element.m_Type == Type::Struct)
						{
							element.m_SizeBytes = element.m_ExtraData->as<Struct&>().CalcStructSize(offset + size);
							size += element.m_SizeBytes;
						}
						else
						{
							size += GetSize(element.m_Type);
						}

					}

					return size;
				}

				uint32_t SetConstChildSizes(uint32_t dataOffset)
				{
					uint32_t size = 0;
					for (Element& el : m_Elements)
					{
						assert("Struct is meant to be const" && el.m_Type != Type::String);
						el.m_DataOffset = size + dataOffset;

						if (el.m_Type == Type::Struct)
						{
							el.m_SizeBytes = el.m_ExtraData->as<Struct>().SetConstChildSizes(el.m_DataOffset);
						}
						else if (el.m_Type == Type::Array)
						{
							assert("Array should not be in type template struct" && false);
						}
						else if (el.m_Type == Type::Pointer)
						{
							el.m_SizeBytes = sizeof(uint32_t);
						}
						else
						{
							el.m_SizeBytes = GetSize(el.m_Type);
						}

						size += el.m_SizeBytes;
					}

					return size;
				}

				uint32_t SetChildSizes(uint32_t absOffset, std::vector<uint32_t>& offsets)
				{
					uint32_t size = 0;
					for (int i = 0; i < m_Elements.size(); i++)
					{
						Element& element = m_Elements[i];
						uint32_t offset = offsets[i];

						element.m_SizeBytes = offsets[i + 1] - offset;
						element.m_DataOffset = absOffset + offset;

						if (element.m_Type == Type::Struct)
						{
							element.m_ExtraData->as<Struct>().SetConstChildSizes(element.m_DataOffset);
						}

						size += element.m_SizeBytes;
					}

					return size;
				}

				std::vector<Element> m_Elements;

				//std::tuple<std::string, uint32_t> m_CachedElement;
			};

			struct Array : public Element::ExtraDataBase
			{
				Array() = default;

				Array(Element::ExtraDataBase* other)
				{
					Array& arrayData = other->as<Array>();

					m_Size = arrayData.m_Size;
					m_ConstElementSize = arrayData.m_ConstElementSize;
					m_Stride = arrayData.m_Stride;
					m_TypeTemplate = Element(arrayData.m_TypeTemplate);
					if (arrayData.m_ElementOffsets.has_value())
					{
						m_ElementOffsets.emplace(arrayData.m_ElementOffsets->begin(), arrayData.m_ElementOffsets->end());
					}
				}

				Array(Type type, uint32_t size, bool constElementSize, std::optional<std::vector<uint32_t>> elementOffsets = std::nullopt)
					:
					m_Size(size),
					m_TypeTemplate({ "", type, 0 }),
					m_ConstElementSize(constElementSize),
					m_ElementOffsets(elementOffsets),
					m_Stride(-1)
				{
					assert("No element offsets were given!" && constElementSize);
				}

				uint32_t m_Size;
				Element m_TypeTemplate;
				bool m_ConstElementSize;

				//Only used if the array type is not constant size
				std::optional<std::vector<uint32_t>> m_ElementOffsets;
				//The size in bytes of the type template
				int m_Stride;
			};
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