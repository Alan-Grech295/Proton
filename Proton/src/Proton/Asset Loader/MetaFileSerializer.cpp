#include "ptpch.h"
#include "MetaFileSerializer.h"
#include <filesystem>
#include <fstream>

#include <string>
#include <vector>
#include "Proton\Core\Log.h"
#include <unordered_map>
#include <assert.h>
#include <optional>
//#include "AssetSerializer.h"


namespace Proton
{
	namespace Meta
	{
		struct ExtraData
		{
			struct Struct : public Element::ExtraDataBase
			{
				//If no meta file is provided then it is assumed that this struct is an array template
				Struct(MetaFile* metaFile, uint32_t numChildren, std::optional<std::string> parentStructTag = std::nullopt)
					:
					m_MetaFile(metaFile),
					m_StructChildren(numChildren)
				{
					if (metaFile)
						m_StructTag = parentStructTag.value_or("") + metaFile->GetNextStructTag() + "|";

					m_Elements.reserve(numChildren);
				}

				//WARN: Setting num elements after assigning elements can cause malfunctions
				void SetNumElements(uint32_t numChildren)
				{
					m_Elements.reserve(numChildren);
					m_StructChildren = numChildren;
				}

				Element& Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset, uint32_t numStructChildren = 0)
				{
					assert("Adding more children than capacity!" && (m_Elements.size() < m_StructChildren));
					//If data offset has no value then it is assumed that the element is used as an array type template
					if (dataOffset.has_value())
					{
						m_MetaFile->AddStructElement(name, type, dataOffset.value(), numStructChildren, m_StructTag, m_Elements);
					}
					else
					{
						m_Elements.push_back(Element(name, type, 0, nullptr, "", numStructChildren));
					}

					return m_Elements.back();
				}

				std::vector<Element> m_Elements;
				std::string m_StructTag;
				//Meta file reference to add any child elements to element locator
				MetaFile* m_MetaFile;

				//Used for validity checks
				uint32_t m_StructChildren;
			};

			struct Array : public Element::ExtraDataBase
			{
				Array() = default;
				Array(Type type, uint32_t size, bool constElementSize, std::optional<std::vector<uint32_t>> elementOffsets = std::nullopt)
					:
					m_Size(size),
					m_TypeTemplate({ "", type, 0 }),
					m_constElementSize(constElementSize),
					m_ElementOffsets(elementOffsets)
				{
					assert("No element offsets were given!" && constElementSize);
				}

				uint32_t m_Size;
				Element m_TypeTemplate;
				bool m_constElementSize;

				//Only used if the array type is not constant size
				std::optional<std::vector<uint32_t>> m_ElementOffsets;
			};
		};

		/*Element::Element(const std::string& name)
			:
			m_Name(name),
			m_Type(Type::Struct)
		{
			m_ExtraData = new ExtraData::Struct();
		}*/

		Element::Element(const char* name, Type type, uint32_t dataOffset, MetaFile* metaFile, std::string parentStructTag, uint32_t numStructChildren)
			:
			m_Name(name),
			m_Type(type),
			m_ExtraData(nullptr),
			m_DataOffset(dataOffset)
		{
			if (type == Type::Struct)
			{
				//assert("Struct cannot have no children" && numStructChildren > 0);
				m_ExtraData = new ExtraData::Struct(metaFile, numStructChildren, parentStructTag.empty() ? std::nullopt : std::optional<std::string>(parentStructTag));
			}
			else if (type == Type::Array)
			{
				m_ExtraData = new ExtraData::Array(Type::None, 0, true);
			}
		}

		Element::Element(std::string name, Type type, uint32_t dataOffset, MetaFile* metaFile, std::string parentStructTag, uint32_t numStructChildren)
			:
			m_Type(type),
			m_ExtraData(nullptr),
			m_DataOffset(dataOffset)
		{
			m_Name = new char[name.length() + 1];
			memcpy(const_cast<char*>(m_Name), name.c_str(), name.length() + 1);
			if (type == Type::Struct)
			{
				assert("Struct cannot have no children" && numStructChildren == 0);
				m_ExtraData = new ExtraData::Struct(metaFile, numStructChildren, parentStructTag.empty() ? std::nullopt : std::optional<std::string>(parentStructTag));
			}
			else if (type == Type::Array)
			{
				m_ExtraData = new ExtraData::Array(Type::None, 0, true);
			}
		}

		/*Element::Element(const std::string& name, Type arrayType, uint32_t size, bool constElementSize)
			:
			m_Name(name),
			m_Type(Type::Array)
		{
			m_ExtraData = new ExtraData::Array(arrayType, size, constElementSize);
		}*/

		std::string Element::ToString()
		{
			if (m_Type == Type::Struct)
			{
				ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);

				std::string str = "Struct: " + std::string(m_Name) + ":\n";
				for (Element& e : data.m_Elements)
				{
					str += e.ToString() + "\n";
				}
				str += "End struct " + std::string(m_Name);

				return str;
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& data = static_cast<ExtraData::Array&>(*m_ExtraData);
				return std::string(m_Name) + ", (Array) Type: " + data.m_TypeTemplate.ToString() + ", Size: " + std::to_string(data.m_Size);
			}

			return std::string(m_Name) + ", " + typeToString[m_Type];
		}

		void Element::ToBytes(std::vector<byte>& bytes)
		{
			/*Value Elements: Name, Type, Data Offset,
			  Struct Elements: Name, Type, Data Offset, Num elements, {Elements}
			  Array Elements: Name, Type, Data Offset, Element Type, Size, Constant element size*/

			//				Name Length			Name					Type			Data Offset
			uint32_t size = sizeof(uint32_t) + strlen(m_Name) + sizeof(uint32_t) + sizeof(uint32_t);

			bytes.resize(bytes.size() + size);

			byte* ptr = &bytes.back() - size + 1;

			//Data required
			uint32_t nameLength = strlen(m_Name);
			uint32_t type = (uint32_t)m_Type;

			COPY_DATA(ptr, &nameLength, sizeof(uint32_t));
			COPY_DATA(ptr, m_Name, nameLength);
			COPY_DATA(ptr, &type, sizeof(uint32_t));
			COPY_DATA(ptr, &m_DataOffset, sizeof(uint32_t));

			if (m_Type == Type::Struct)
			{
				ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);

				bytes.resize(bytes.size() + sizeof(uint32_t));

				ptr = &bytes.back() - sizeof(uint32_t) + 1;

				//Data required
				uint32_t numElements = structData.m_Elements.size();

				COPY_DATA(ptr, &numElements, sizeof(uint32_t));

				for (Element& e : structData.m_Elements)
					e.ToBytes(bytes);
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
				//			Type Data										Size			Constant Size											Num Offsets							{Offsets}
				size = arrayData.m_TypeTemplate.CalculateSizeInArray() + sizeof(uint32_t) + sizeof(boolean) + (arrayData.m_constElementSize ? 0 : sizeof(uint32_t) + sizeof(uint32_t) * arrayData.m_ElementOffsets.value().size());

				bytes.resize(bytes.size() + size);

				byte* ptr = &bytes.back() - size + 1;

				auto [elementData, dataSize] = arrayData.m_TypeTemplate.GetDataForArray();
				COPY_DATA(ptr, elementData, dataSize);
				COPY_DATA(ptr, &arrayData.m_Size, sizeof(uint32_t));
				COPY_DATA(ptr, &arrayData.m_constElementSize, sizeof(bool));

				if (!arrayData.m_constElementSize)
				{
					uint32_t numOffsets = arrayData.m_ElementOffsets.value().size();
					COPY_DATA(ptr, &numOffsets, sizeof(uint32_t));
					COPY_DATA(ptr, arrayData.m_ElementOffsets.value().data(), numOffsets * sizeof(uint32_t));
				}

				delete[] elementData;
			}
		}

		Element& Element::Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset, uint32_t numStructChildren)
		{
			assert("Element is not a struct!" && m_Type == Type::Struct);
			ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);
			return data.Add(name, type, dataOffset, numStructChildren);
		}

		/*void Element::Add(Element element)
		{
			assert("Element is not a struct!" && m_Type == Type::Struct);
			ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);
			data.m_MetaFileRef.AddStructElement(element.m_Name, data.m_Elements.size(), data.m_StructTag);
			data.m_Elements.push_back(element);
		}*/

		Element& Element::operator[](const std::string& name)
		{
			assert("Element is not a struct!" && m_Type == Type::Struct);
			ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);

			std::string search = structData.m_StructTag + name;

			return (*structData.m_MetaFile)[search];
		}

		Element& Element::GetType()
		{
			assert("Element is not an array!" && m_Type == Type::Array);
			ExtraData::Array& data = static_cast<ExtraData::Array&>(*m_ExtraData);
			return data.m_TypeTemplate;
		}

		//A size of 0 means no size is set
		Element& Element::SetSize(uint32_t size)
		{
			ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
			arrayData.m_Size = size;
			return *this;
		}

		Element& Element::SetType(Type type)
		{
			ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
			arrayData.m_TypeTemplate = Element(nullptr, type, 0, nullptr, "", 5);
			return *this;
		}

		Element& Element::SetElementOffsets(std::vector<uint32_t> offsets)
		{
			ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
			arrayData.m_constElementSize = false;
			arrayData.m_ElementOffsets = std::move(offsets);
			return *this;
		}

		uint32_t Element::CalculateSizeInArray()
		{
			if (m_Type == Type::Struct)
			{
				//Type, size, {Element types}
				uint32_t size = 2 * sizeof(uint32_t);
				ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);

				for (Element& e : data.m_Elements)
				{
					size += e.CalculateSizeInArray();
				}

				return size;
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& data = static_cast<ExtraData::Array&>(*m_ExtraData);

				//Type, Elements Type, size
				return 2 * sizeof(uint32_t) + data.m_TypeTemplate.CalculateSizeInArray();
			}
			else
			{
				//Only type is returned
				return sizeof(uint32_t);
			}
		}

		//Gets the byte data when stored for array type template (i.e no name is stored)
		//TODO: Add non-constant size arrays
		std::pair<byte*, uint32_t> Element::GetDataForArray()
		{
			if (m_Type == Type::Struct)
			{
				//Type, Size, {Element types}
				ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);
				uint32_t size = 2 * sizeof(uint32_t);

				for (Element& e : structData.m_Elements)
				{
					size += e.CalculateSizeInArray();
				}

				byte* bytes = new byte[size];
				byte* ptr = bytes;

				//Data to copy
				uint32_t numElements = structData.m_Elements.size();
				uint32_t type = (uint32_t)Type::Struct;

				COPY_DATA(ptr, &type, sizeof(uint32_t));
				COPY_DATA(ptr, &numElements, sizeof(uint32_t));

				for (Element& e : structData.m_Elements)
				{
					auto [data, dataSize] = e.GetDataForArray();
					COPY_DATA(ptr, data, dataSize);
					delete[] data;
				}

				return { bytes, size };
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);

				//Type, Elements Type, size
				uint32_t size = arrayData.m_TypeTemplate.CalculateSizeInArray() + 2 * sizeof(uint32_t);
				byte* data = new byte[size];
				byte* ptr = data;

				uint32_t type = (uint32_t)Type::Array;
				COPY_DATA(ptr, &type, sizeof(uint32_t));
				auto [typeData, dataSize] = arrayData.m_TypeTemplate.GetDataForArray();
				COPY_DATA(ptr, typeData, dataSize);
				COPY_DATA(ptr, &arrayData.m_Size, sizeof(uint32_t));

				delete[] typeData;

				return { data, size };
			}
			else
			{
				//Only type is returned
				uint32_t* type = new uint32_t((uint32_t)m_Type);
				return { (byte*)type, sizeof(uint32_t) };
			}
		}

		MetaFile& MetaFileSerializer::DeserializeMetaFile(const std::string& path)
		{
			std::ifstream inStream(path, std::ios::out | std::ios::binary);

			if (!inStream)
			{
				PT_CORE_ERROR("Cannot open file {0}!", path);
			}

			std::filebuf* fileBuf = inStream.rdbuf();
			uint64_t size = fileBuf->pubseekoff(0, inStream.end, inStream.in);
			fileBuf->pubseekpos(0, inStream.in);

			byte* buffer = new byte[size];
			byte* ptr = buffer;

			fileBuf->sgetn((char*)buffer, size);

			inStream.close();

			std::string byteStr;
			int lastMod = 1;
			for (int i = 0; i < size; i++)
			{
				byte b = buffer[i];
				byteStr += std::to_string(b) + ' ';
				if (byteStr.length() % 50 < lastMod)
					byteStr += '\n';

				lastMod = byteStr.length() % 50;
			}
			byteStr += "\n\n";
			PT_CORE_TRACE(byteStr);

			MetaFile& metaFile = *new MetaFile(*(uint32_t*)ptr);
			ptr += sizeof(uint32_t);

			while (ptr - buffer < size)
			{
				ReadElement(ptr, metaFile);
			}

			delete[] buffer;

			return metaFile;
		}

		void MetaFileSerializer::SerializeMetaFile(const std::string& path, MetaFile& file)
		{
			std::vector<byte> bytes;

			uint32_t numElements = file.m_Elements.size();
			
			bytes.resize(sizeof(uint32_t));
			memcpy(bytes.data(), &numElements, sizeof(uint32_t));

			for (Element& e : file.m_Elements)
			{
				e.ToBytes(bytes);
			}

			std::string byteStr;
			int lastMod = 1;
			for (byte b : bytes)
			{
				byteStr += std::to_string(b) + ' ';
				if (byteStr.length() % 50 < lastMod)
					byteStr += '\n';

				lastMod = byteStr.length() % 50;
			}
			byteStr += "\n\n";
			PT_CORE_TRACE(byteStr);

			std::ofstream outStream(path, std::ios::out | std::ios::binary);

			outStream.write((char*)bytes.data(), bytes.size());
			outStream.close();

			bytes.clear();
		}

		//Reads the element from the bytes
		Element MetaFileSerializer::ReadElement(byte*& data, MetaFile& file, Element::ExtraDataBase* structData)
		{
			/*Value: Name, Type, Data Offset
			  Struct: Name, Type, Data Offset, Num Elements, {Elements}
			  Array: Name, Type, Data Offset, Type Template, Size, Constant Element Size, (Num Offsets, {Offsets})*/
			uint32_t nameLength = *(uint32_t*)data; data += sizeof(uint32_t);
			std::string name = std::string((char*)data, nameLength); data += nameLength;
			Type type = (Type) * (uint32_t*)data; data += sizeof(uint32_t);
			uint32_t dataOffset = *(uint32_t*)data; data += sizeof(uint32_t);

			Element& element = structData ? ((ExtraData::Struct*)structData)->Add(name, type, dataOffset) : file.Add(name, type, dataOffset);

			if (type == Type::Struct)
			{
				ExtraData::Struct* structData = static_cast<ExtraData::Struct*>(element.m_ExtraData);
				uint32_t numElements = *(uint32_t*)data; data += sizeof(uint32_t);
				structData->SetNumElements(numElements); 

				for (int i = 0; i < numElements; i++)
					ReadElement(data, file, structData);
			}
			else if (type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*element.m_ExtraData);

				arrayData.m_TypeTemplate = ReadArrayElement(data);
				arrayData.m_Size = *(uint32_t*)data; data += sizeof(uint32_t);
				arrayData.m_constElementSize = *(bool*)data; data += sizeof(bool);

				if (!arrayData.m_constElementSize)
				{
					uint32_t numOffsets = *(uint32_t*)data; data += sizeof(uint32_t);
					arrayData.m_ElementOffsets = std::vector<uint32_t>((uint32_t*)data, (uint32_t*)(data + numOffsets * sizeof(uint32_t)));
					data += numOffsets * sizeof(uint32_t);
				}
			}

			return element;
		}

		//Reads the element data when stored for the use of an array (i.e no name is saved)
		//TODO: Support non-const size arrays
		Element& MetaFileSerializer::ReadArrayElement(byte*& data)
		{
			Type type = (Type) * (uint32_t*)data; data += sizeof(uint32_t);
			Element& element = Element("", type, 0);

			if (type == Type::Struct)
			{
				uint32_t numElements = *(uint32_t*)data; data += sizeof(uint32_t);
				ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*element.m_ExtraData);
				structData.SetNumElements(numElements);

				for (int i = 0; i < numElements; i++)
					structData.m_Elements.push_back(ReadArrayElement(data));
			}
			else if (type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*element.m_ExtraData);

				arrayData.m_TypeTemplate = ReadArrayElement(data);
				arrayData.m_Size = *(uint32_t*)data; data += sizeof(uint32_t);
			}
			
			return element;
		}
	}
}