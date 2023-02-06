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
		Element::Element(const char* name, Type type, uint32_t dataOffset)
			:
			m_Name(name),
			m_Type(type),
			m_ExtraData(nullptr),
			m_DataOffset(dataOffset)
		{
			if (type == Type::Struct)
				m_ExtraData = new ExtraData::Struct();
			else if (type == Type::Array)
				m_ExtraData = new ExtraData::Array(Type::None, 0, true);
		}

		Element::Element(std::string name, Type type, uint32_t dataOffset)
			:
			m_Type(type),
			m_ExtraData(nullptr),
			m_DataOffset(dataOffset)
		{
			//Copies the name
			m_Name = new char[name.length() + 1];
			memcpy(const_cast<char*>(m_Name), name.c_str(), name.length() + 1);

			if (type == Type::Struct)
				m_ExtraData = new ExtraData::Struct();
			else if (type == Type::Array)
				m_ExtraData = new ExtraData::Array(Type::None, 0, true);
		}

		Element::Element(const Element& element)
			:
			m_Type(element.m_Type),
			m_DataOffset(element.m_DataOffset),
			m_Name(element.m_Name),
			m_SizeBytes(element.m_SizeBytes),
			m_ExtraData(nullptr)
		{
			if (m_Type == Type::Struct)
				m_ExtraData = new ExtraData::Struct(element.m_ExtraData);
			else if (m_Type == Type::Array)
				m_ExtraData = new ExtraData::Array(element.m_ExtraData);
		}

		//For debug only
		std::string Element::ToString()
		{
			if (m_Type == Type::Struct)
			{
				ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);

				std::string str = "Struct: " + std::string(m_Name) + ":\n";
				//Loops over all child elements
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

		//Converts the element to bytes
		//POSSIBLE OPTIMIZATION: Calculate the size of all the data and allocate at once
		//						 rather than use a vector
		void Element::ToBytes(std::vector<byte>& bytes)
		{
			/*Value Elements:	Name, Type, Data Offset, Size Bytes
			  Struct Elements:	Name, Type, Data Offset, Size Bytes, Num elements, {Elements}
			  Array Elements:	Name, Type, Data Offset, Size Bytes, Element Type, Size, Constant element size, {If not const size and array type template : Child Stride}*/

			//The size of the meta element in bytes
			uint32_t size = CalculateSizeBytes();
			//Gets the offset of the starting point of the data
			//(not using pointer as vector can be reallocated)
			uint32_t offset = (uint32_t)bytes.size();

			bytes.resize(bytes.size() + size);

			//Data required
			uint32_t nameLength = (uint32_t)strlen(m_Name);
			uint32_t type = (uint32_t)m_Type;

			COPY_DATA_OFFSET(bytes.data(), offset, &nameLength, sizeof(uint32_t));
			COPY_DATA_OFFSET(bytes.data(), offset, m_Name, nameLength);
			COPY_DATA_OFFSET(bytes.data(), offset, &type, sizeof(uint32_t));
			COPY_DATA_OFFSET(bytes.data(), offset, &m_DataOffset, sizeof(uint32_t));
			COPY_DATA_OFFSET(bytes.data(), offset, &m_SizeBytes, sizeof(uint32_t));

			if (m_Type == Type::Struct)
			{
				ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);

				//Data required
				uint32_t numElements = (uint32_t)structData.m_Elements.size();

				COPY_DATA_OFFSET(bytes.data(), offset, &numElements, sizeof(uint32_t));

				for (Element& e : structData.m_Elements)
					e.ToBytes(bytes);
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);

				//Gets the data of the type template
				std::vector<byte> typeData = arrayData.m_TypeTemplate.GetDataForArray();

				COPY_DATA_OFFSET(bytes.data(), offset, typeData.data(), typeData.size());
				COPY_DATA_OFFSET(bytes.data(), offset, &arrayData.m_Size, sizeof(uint32_t));
				COPY_DATA_OFFSET(bytes.data(), offset, &arrayData.m_ConstElementSize, sizeof(bool));
				COPY_DATA_OFFSET(bytes.data(), offset, &arrayData.m_Stride, sizeof(int));

				//If the array template is not of const size the element offsets need to be saved
				if (!arrayData.m_ConstElementSize)
				{
					uint32_t numOffsets = (uint32_t)arrayData.m_ElementOffsets.value().size();
					COPY_DATA_OFFSET(bytes.data(), offset, &numOffsets, sizeof(uint32_t));
					COPY_DATA_OFFSET(bytes.data(), offset, arrayData.m_ElementOffsets.value().data(), numOffsets * sizeof(uint32_t));
				}
			}
		}

		//Add element to struct element
		Element& Element::Add(const std::string& name, Type type, std::optional<uint32_t> dataOffset)
		{
			assert("Element is not a struct!" && m_Type == Type::Struct);
			ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);
			return data.Add(name, type, dataOffset);
		}

		//Find element in struct element
		Element& Element::operator[](const std::string& name)
		{
			assert("Element is not a struct!" && m_Type == Type::Struct);
			ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);

			return structData[name];
		}

		bool Element::Has(const std::string& name) const
		{
			assert("Element is not a struct!" && m_Type == Type::Struct);
			ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);

			return structData.Has(name);
		}

		//Get the type template of an array element
		Element& Element::GetType()
		{
			assert("Element is not an array!" && m_Type == Type::Array);
			ExtraData::Array& data = static_cast<ExtraData::Array&>(*m_ExtraData);
			return data.m_TypeTemplate;
		}

		//Set the size of an array element
		//A size of 0 means no size is set
		Element& Element::SetSize(uint32_t size)
		{
			assert("Element is not an array!" && m_Type == Type::Array);
			ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
			arrayData.m_Size = size;
			return *this;
		}

		//Set the type template of an array element
		Element& Element::SetType(Type type)
		{
			assert("Element is not an array!" && m_Type == Type::Array);
			ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
			arrayData.m_TypeTemplate = Element(nullptr, type, 0);
			return *this;
		}

		//Set the element offsets of an array element
		Element& Element::SetElementOffsets(std::vector<uint32_t> offsets)
		{
			assert("Element is not an array!" && m_Type == Type::Array);
			ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
			//If element offsets are given it is assumed that the
			//array type template is not of const size
			arrayData.m_ConstElementSize = false;
			arrayData.m_ElementOffsets = std::move(offsets);
			return *this;
		}

		//The data in bytes when stored in an array element
		uint32_t Element::CalculateSizeInArray()
		{
			if (m_Type == Type::Struct)
			{
				//Type, size, {Element types}

				uint32_t size = 2 * sizeof(uint32_t);
				ExtraData::Struct& data = static_cast<ExtraData::Struct&>(*m_ExtraData);

				for (Element& e : data.m_Elements)
				{
					size += sizeof(uint32_t) + strlen(e.m_Name) + e.CalculateSizeInArray();
				}

				return size;
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& data = static_cast<ExtraData::Array&>(*m_ExtraData);

				//Type, Elements Type, size, stride
				return sizeof(uint32_t) + data.m_TypeTemplate.CalculateSizeInArray() + sizeof(uint32_t) + sizeof(int);
			}
			else
			{
				//Only type is returned
				return sizeof(uint32_t);
			}
		}

		//Size of bytes when saved
		uint32_t Element::CalculateSizeBytes()
		{
			//				Name Length			Name					Type			Data Offset         Size Bytes
			uint32_t size = sizeof(uint32_t) + (uint32_t)strlen(m_Name) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t);
			
			if (m_Type == Type::Struct)
			{
				//Num elements
				size += sizeof(uint32_t);
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);
				//			Type Data										Size			Constant Size		Stride												Num Offsets							{Offsets}
				size += arrayData.m_TypeTemplate.CalculateSizeInArray() + sizeof(uint32_t) + sizeof(boolean) + sizeof(int) + (arrayData.m_ConstElementSize ? 0 : sizeof(uint32_t) + sizeof(uint32_t) * arrayData.m_ElementOffsets.value().size());
			}

			return size;
		}

		//Gets the byte data when stored for array type template (i.e no name and data offset are stored)
		//TODO: Add non-constant size arrays
		std::vector<byte> Element::GetDataForArray()
		{
			std::vector<byte> bytes;
			bytes.resize(sizeof(uint32_t));

			//Type being stored
			uint32_t* type = new uint32_t((uint32_t)m_Type);
			memcpy(bytes.data(), type, sizeof(uint32_t));

			//Start data offset after the type (4 bytes)
			uint32_t offset = sizeof(uint32_t);

			if (m_Type == Type::Struct)
			{
				//Size, {Elements}

				ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*m_ExtraData);

				bytes.resize(bytes.size() + sizeof(uint32_t));

				uint32_t numElements = (uint32_t)structData.m_Elements.size();

				COPY_DATA_OFFSET(bytes.data(), offset, &numElements, sizeof(uint32_t));

				for (Element& e : structData.m_Elements)
				{
					//Saves the name and data for the array
					std::vector<byte> elData = e.GetDataForArray();
					uint32_t nameLength = (uint32_t)strlen(e.m_Name);

					bytes.resize(bytes.size() + sizeof(uint32_t) + nameLength + elData.size());

					COPY_DATA_OFFSET(bytes.data(), offset, &nameLength, sizeof(uint32_t));
					COPY_DATA_OFFSET(bytes.data(), offset, e.m_Name, nameLength);
					COPY_DATA_OFFSET(bytes.data(), offset, elData.data(), elData.size());
				}
			}
			else if (m_Type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*m_ExtraData);

				std::vector<byte> typeData = arrayData.m_TypeTemplate.GetDataForArray();
				//Elements Type, Size, Stride
				bytes.resize(bytes.size() + typeData.size() + sizeof(uint32_t) + sizeof(int));

				COPY_DATA_OFFSET(bytes.data(), offset, typeData.data(), typeData.size());
				COPY_DATA_OFFSET(bytes.data(), offset, &arrayData.m_Size, sizeof(uint32_t));
				COPY_DATA_OFFSET(bytes.data(), offset, &arrayData.m_Stride, sizeof(int));
			}

			return bytes;
		}

		MetaFile& MetaFileSerializer::DeserializeMetaFile(const std::string& path)
		{
			//Reads the file into a buffer
			std::ifstream inStream(path, std::ios::out | std::ios::binary);

			if (!inStream)
				PT_CORE_ERROR("Cannot open file {0}!", path);

			std::filebuf* fileBuf = inStream.rdbuf();
			uint64_t size = fileBuf->pubseekoff(0, inStream.end, inStream.in);
			fileBuf->pubseekpos(0, inStream.in);

			byte* buffer = new byte[size];
			byte* ptr = buffer;

			fileBuf->sgetn((char*)buffer, size);

			inStream.close();
			//End file reading

			//Displays the read bytes
			/*std::string byteStr;
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
			PT_CORE_TRACE(byteStr);*/

			//Creates a new meta file, specifying how many elements it has
			MetaFile& metaFile = *new MetaFile(*(uint32_t*)ptr);
			ptr += sizeof(uint32_t);

			//Reads the elements
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

			//Adds the number of elements in the meta file
			uint32_t numElements = (uint32_t)file.m_Elements.size();
			
			bytes.resize(sizeof(uint32_t));
			memcpy(bytes.data(), &numElements, sizeof(uint32_t));

			//Serializes the elements
			for (Element& e : file.m_Elements)
			{
				e.ToBytes(bytes);
			}

			//Displays the saved bytes
			/*std::string byteStr;
			int lastMod = 1;
			for (byte b : bytes)
			{
				byteStr += std::to_string(b) + ' ';
				if (byteStr.length() % 50 < lastMod)
					byteStr += '\n';

				lastMod = byteStr.length() % 50;
			}
			byteStr += "\n\n";
			PT_CORE_TRACE(byteStr);*/

			//Writes the bytes to a file
			std::ofstream outStream(path, std::ios::out | std::ios::binary);

			outStream.write((char*)bytes.data(), bytes.size());
			outStream.close();
		}

		//Reads the element from the bytes
		Element MetaFileSerializer::ReadElement(byte*& data, Addable& addable)
		{
			/*Value:	Name, Type, Data Offset, Size Bytes
			  Struct:	Name, Type, Data Offset, Size Bytes, Num Elements, {Elements}
			  Array:	Name, Type, Data Offset, Size Bytes, Type Template, Size, Constant Element Size, (Num Offsets, {Offsets}), {If not const size and array type template : Child Stride}*/
			
			//Basic element data
			uint32_t nameLength = *(uint32_t*)data; data += sizeof(uint32_t);
			std::string name = std::string((char*)data, nameLength); data += nameLength;
			Type type = (Type) * (uint32_t*)data; data += sizeof(uint32_t);
			uint32_t dataOffset = *(uint32_t*)data; data += sizeof(uint32_t);
			uint32_t sizeBytes = *(uint32_t*)data; data += sizeof(uint32_t);

			//Creates an element from the read data and adds it to an addable type (Asset or Element)
			Element& element = addable.Add(name, type, dataOffset);
			element.m_SizeBytes = sizeBytes;

			if (type == Type::Struct)
			{
				ExtraData::Struct* structData = static_cast<ExtraData::Struct*>(element.m_ExtraData);
				uint32_t numElements = *(uint32_t*)data; data += sizeof(uint32_t);

				//Read the struct elements
				for (int i = 0; i < numElements; i++)
					ReadElement(data, element);
			}
			else if (type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*element.m_ExtraData);

				//Reads the type template
				arrayData.m_TypeTemplate = ReadArrayElement(data);

				arrayData.m_Size = *(uint32_t*)data; data += sizeof(uint32_t);
				arrayData.m_ConstElementSize = *(bool*)data; data += sizeof(bool);
				arrayData.m_Stride = *(int*)data; data += sizeof(int);

				if (!arrayData.m_ConstElementSize)
				{
					//Read the element offsets
					uint32_t numOffsets = *(uint32_t*)data; data += sizeof(uint32_t);
					arrayData.m_ElementOffsets = std::vector<uint32_t>((uint32_t*)data, (uint32_t*)(data + numOffsets * sizeof(uint32_t)));
					data += numOffsets * sizeof(uint32_t);
				}
			}

			return element;
		}

		//Reads the element data when stored for the use of an array (i.e no name or data offset are saved)
		//TODO: Support non-const size arrays
		Element MetaFileSerializer::ReadArrayElement(byte*& data, bool readName)
		{
			std::string name = "";
			if (readName)
			{
				//Reads the name of the element 
				//(If element is a child of a struct which is the type template of an array)
				uint32_t nameLength = *(uint32_t*)data; data += sizeof(uint32_t);
				name = std::string((char*)data, nameLength); data += nameLength;
			}

			Type type = (Type) * (uint32_t*)data; data += sizeof(uint32_t);
			Element element = Element(name, type, 0);

			if (type == Type::Struct)
			{
				uint32_t numElements = *(uint32_t*)data; data += sizeof(uint32_t);
				ExtraData::Struct& structData = static_cast<ExtraData::Struct&>(*element.m_ExtraData);

				//Reads the element data (including name) and adds them to the struct
				for (int i = 0; i < numElements; i++)
				{
					structData.m_Elements.push_back(ReadArrayElement(data, true));
				}
			}
			else if (type == Type::Array)
			{
				ExtraData::Array& arrayData = static_cast<ExtraData::Array&>(*element.m_ExtraData);

				//Reads the array type template and size
				arrayData.m_TypeTemplate = ReadArrayElement(data);
				arrayData.m_Size = *(uint32_t*)data; data += sizeof(uint32_t);
				arrayData.m_Stride = *(int*)data; data += sizeof(int);
			}
			
			return element;
		}
	}
}