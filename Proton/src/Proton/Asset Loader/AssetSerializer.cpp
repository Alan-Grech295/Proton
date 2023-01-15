#include "ptpch.h"
#include "AssetSerializer.h"
#include "MetaFileSerializer.cpp"

namespace Proton
{
	TypeElement::TypeElement(std::initializer_list<Element> elements)
		:
		m_Type(Type::Struct),
		m_Data(new Data::Struct())
	{
		//TODO: Check struct child names
		std::vector<Element>& el = m_Data->as<Data::Struct&>().m_Elements;
		el = elements;
	}

	TypeElement::TypeElement(Type type)
		:
		m_Type(type),
		m_Data(new Data::Value())
	{
		if (type == Type::Struct)
			m_Data = new Data::Struct();
		else if (type == Type::Array)
			m_Data = new Data::Array();
	}

	TypeElement::TypeElement(Type type, byte* data)
		:
		m_Type(type),
		m_Data(new Data::Value(data, type))
	{
	}

	//Adds an element to an array
	Element& TypeElement::Add(TypeElement entry)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		Data::Array& arrayData = static_cast<Data::Array&>(*m_Data);
		assert("Array does not have type template!" && arrayData.m_TypeTemplate.has_value());
		assert("Element does not match array template!" && entry == arrayData.m_TypeTemplate.value());

		if (arrayData.m_Size.has_value())
			assert("Array element size exceeded!" && arrayData.m_Elements.size() < arrayData.m_Size.value());

		Element e = entry;
		arrayData.Add(std::move(e));
		return arrayData.m_Elements.back();
	}

	//Sets the type template of an array
	TypeElement& Proton::TypeElement::SetType(TypeElement type)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		Data::Array& data = static_cast<Data::Array&>(*m_Data);
		assert("Cannot change array type after elements are added!" && data.m_Elements.empty());

		//Check if array contains type template whose size is not consistent
		data.m_constElementSize = type.m_Type != Type::String;
		if (data.m_constElementSize)
		{
			if (type.m_Type == Type::Struct)
			{
				Data::Struct& structData = type.m_Data->as<Data::Struct&>();
				structData.m_ConstSize = true;
				structData.SetConstElementSize();
				data.m_constElementSize = structData.ConstantSize();
			}
			else if (type.m_Type == Type::Array)
			{
				Data::Array& arrayData = type.m_Data->as<Data::Array&>();
				//Does not allow the type template to have non-const elements
				arrayData.m_CanBeNonConst = false;

				//If m_Size has a value then size restrictions need to be set
				if (data.m_Size.has_value() || !data.m_CanBeNonConst)
					arrayData.m_Size = 0;
			}
		}

		assert("This array cannot have a non-const type template!" && data.m_CanBeNonConst || data.m_constElementSize);

		data.m_TypeTemplate = std::move(type);

		return *this;
	}

	//Gets the type of the type template
	TypeElement& Proton::TypeElement::GetType()
	{
		return m_Data->as<Data::Array&>().m_TypeTemplate.value();
	}

	//Sets the size of an array
	TypeElement& Proton::TypeElement::SetSize(uint32_t size)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		Data::Array& data = m_Data->as<Data::Array&>();
		assert("Cannot set array size after elements are added!" && data.m_Elements.empty());
		assert("Size must be set before assigning array type template!" && !(data.m_TypeTemplate.has_value() && data.m_TypeTemplate->m_Type == Type::Array));
		data.m_Size = size;

		return *this;
	}

	//Gets the size in bytes of the element data
	uint32_t TypeElement::GetSizeInBytes()
	{
		if (m_Type == Type::Struct)
		{
			uint32_t sizeBytes = 0;

			//Gets the size of the child elements
			for (Element& e : m_Data->as<Data::Struct&>().m_Elements)
				sizeBytes += e.GetSizeInBytes();

			return sizeBytes;
		}
		else if (m_Type == Type::Array)
		{
			Data::Array& arrayData = m_Data->as<Data::Array&>();

			//If array elements are of const size then you can get 
			//the size of one element and multiply it by the size of the array
			if (arrayData.m_constElementSize)
			{
				return arrayData.m_Elements.back().GetSizeInBytes() * arrayData.m_Size.value_or(arrayData.m_Elements.size());
			}
			else
			{
				//Calculate element offsets
				//if (!arrayData.m_ElementOffsets.has_value()) Check whether they were already calculated?
				arrayData.CalcElementOffsets();

				//Gets the last element offset (the offset of the starting point of the last element)
				//and add the size of the last element to find the size of the array in bytes
				return arrayData.m_ElementOffsets->back() + arrayData.m_Elements.back().GetSizeInBytes();
			}
		}

		switch (m_Type)
		{
		#define X(el) case Type::el: return Map<Type::el>::SizeBytes(m_Data->as<Data::Value&>().m_Data);
			ELEMENT_TYPES
		#undef X
		}
	}

	TypeElement::operator Element()
	{
		Element e = static_cast<Element>(*this);
		return e;
	}

	TypeElement::operator const Element() const
	{
		Element e = static_cast<Element>(*this);
		return e;
	}

	TypeElement::operator Element&()
	{
		Element& e = static_cast<Element&>(*this);
		return e;
	}

	TypeElement::operator const Element&() const
	{
		Element e = static_cast<Element>(*this);
		return e;
	}

	bool TypeElement::Equals(TypeElement& other)
	{
		if (m_Type != other.m_Type)
			return false;

		if (m_Type == Type::Struct)
		{
			Data::Struct& thisData = m_Data->as<Data::Struct>();
			Data::Struct& otherData = other.m_Data->as<Data::Struct>();

			if (thisData.m_Elements.size() != otherData.m_Elements.size())
				return false;

			//Check if the elements of the struct are equal
			for (auto it1 = thisData.m_Elements.begin(), it2 = otherData.m_Elements.begin();
				it1 != thisData.m_Elements.end(); it1++, it2++)
			{
				if (*it1 != *it2)
					return false;
			}

			return true;
		}
		else if (m_Type == Type::Array)
		{
			Data::Array& thisData = m_Data->as<Data::Array>();
			Data::Array& otherData = other.m_Data->as<Data::Array>();

			if (thisData.m_Elements.size() != otherData.m_Elements.size())
				return false;

			//Check if the elements of the array are equal 
			for (auto it1 = thisData.m_Elements.begin(), it2 = otherData.m_Elements.begin();
				it1 != thisData.m_Elements.end(); it1++, it2++)
			{
				if (*it1 != *it2)
					return false;
			}

			return thisData.m_TypeTemplate.value().Equals(otherData.m_TypeTemplate.value());
		}
		else
		{
			return m_Type == other.m_Type;
		}
	}

	const bool TypeElement::Equals(TypeElement& other) const
	{
		if (m_Type != other.m_Type)
			return false;

		if (m_Type != Type::Struct && m_Type != Type::Array)
		{
			return m_Type == other.m_Type;
		}
		else
		{
			if (m_Type == Type::Struct)
			{
				Data::Struct& thisData = m_Data->as<Data::Struct>();
				Data::Struct& otherData = other.m_Data->as<Data::Struct>();

				if (thisData.m_Elements.size() != otherData.m_Elements.size())
					return false;

				//Check if the elements of the struct are equal 
				for (auto it1 = thisData.m_Elements.begin(), it2 = otherData.m_Elements.begin();
					it1 != thisData.m_Elements.end(); it1++, it2++)
				{
					if (*it1 != *it2)
						return false;
				}

				return true;
			}
			else
			{
				Data::Array& thisData = m_Data->as<Data::Array>();
				Data::Array& otherData = other.m_Data->as<Data::Array>();

				if (thisData.m_Elements.size() != otherData.m_Elements.size())
					return false;

				//Check if the elements of the array are equal 
				for (auto it1 = thisData.m_Elements.begin(), it2 = otherData.m_Elements.begin();
					it1 != thisData.m_Elements.end(); it1++, it2++)
				{
					if (*it1 != *it2)
						return false;
				}

				return thisData.m_TypeTemplate.value().Equals(otherData.m_TypeTemplate.value());
			}
		}
	}

	template<typename T>
	T& TypeElement::DataBase::as()
	{
		return static_cast<T&>(*this);
	}

	Element::Element(const char* name, Type type, byte* data)
		:
		TypeElement(type),
		m_Name(name)
	{
		m_Data = new Data::Value(data, type);
	}

	Element::Element(const char* name, Type type)
		:
		TypeElement(type),
		m_Name(name)
	{
	}

	Element& Element::Add(std::initializer_list<Element> elements)
	{
		Data::Struct& structData = m_Data->as<Data::Struct&>();
		for (const Element el : elements)
		{
			assert("Name already exists in struct" && !structData.Contains(el.m_Name));
			if (el.m_Type == Type::Struct)
			{
				el.m_Data->as<Data::Struct&>().m_ConstSize = structData.m_ConstSize;
			}
		}
		int prevSize = structData.m_Elements.size();
		structData.m_Elements.resize(prevSize + elements.size());
		void* copyStart = (void*)(structData.m_Elements.data() + prevSize);
		memcpy(copyStart, elements.begin(), elements.size() * sizeof(Element));

		return *this;
	}

	Element& Element::Add(TypeElement element)
	{
		return ((TypeElement*)this)->Add(static_cast<TypeElement>(element));
	}

	Element& TypeElement::operator[](const std::string& name)
	{
		assert("Element is not a struct!" && m_Type == Type::Struct);
		Data::Struct& structData = static_cast<Data::Struct&>(*m_Data);

		//Find the element with the same name
		for (Element& e : structData.m_Elements)
		{
			if (e.m_Name == name)
				return e;
		}

		return Element::Empty();
	}

	TypeElement& TypeElement::operator[](uint32_t index)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		Data::Array& data = m_Data->as<Data::Array&>();
		assert("Index out of bounds" && index < data.m_Elements.size());

		return data.m_Elements[index];
	}

	//Asset
	Asset::Asset(RawAsset rawAsset)
		:
		m_MetaFile(rawAsset.m_Elements.size())
	{
		//Gets the size of the raw asset elements
		for (Element& e : rawAsset.m_Elements)
			m_DataSize += e.GetSizeInBytes();

		m_Data = new byte[m_DataSize];
		byte* nextData = m_Data;

		//Adds the element data to m_Data and adds the corresponding
		//meta element to the meta file
		for (Element& e : rawAsset.m_Elements)
			AddElement(e, m_Data, nextData, m_MetaFile);
	}

	void Asset::AddElement(Element& element, const byte* dataPtr, byte*& nextDataPtr, Meta::Addable& addable)
	{
		//Creates and adds a meta element from the element
		Meta::Element& metaElement = AddMetaElement(element, addable, (uint32_t)(nextDataPtr - dataPtr));

		if (element.m_Type == Type::Struct)
		{
			//Adds the struct children
			for (Element& e : element.m_Data->as<Data::Struct&>().m_Elements)
				AddElement(e, dataPtr, nextDataPtr, metaElement);
		}
		else if (element.m_Type == Type::Array)
		{
			Data::Array& arrayData = element.m_Data->as<Data::Array&>();

			//Adds the array children element data 
			for (Element& e : arrayData.m_Elements)
				AddArrayElementData(e, nextDataPtr);

			//If the array contains less elements than the specified size then increment the data pointer till the end of the size
			if (arrayData.m_Size.has_value() && *arrayData.m_Size > arrayData.m_Elements.size())
				nextDataPtr += arrayData.m_Elements.back().GetSizeInBytes() * (*arrayData.m_Size - arrayData.m_Elements.size());
		}
		else
		{
			uint32_t dataSize = 0;
			byte* data = element.m_Data->as<Data::Value&>().m_Data;

			//TODO: Have type size ready not find it every time
			switch (element.m_Type)
			{
#define X(el) case Type::el: dataSize = Map<Type::el>::SizeBytes(data); break;
				ELEMENT_TYPES
#undef X
			}

			memcpy(nextDataPtr, data, dataSize);
			nextDataPtr += dataSize;
		}
	}

	void Proton::Asset::AddArrayElementData(Element& element, byte*& nextDataPtr)
	{
		assert("Array data cannot be of variable size!" && !(element.m_Type == Type::Array && !element.m_Data->as<Data::Array>().m_constElementSize));
		if (element.m_Type == Type::Struct)
		{
			//Adds the struct children data 
			for (Element& e : element.m_Data->as<Data::Struct&>().m_Elements)
				AddArrayElementData(e, nextDataPtr);
		}
		else if (element.m_Type == Type::Array)
		{
			Data::Array& arrayData = element.m_Data->as<Data::Array&>();
			//Adds the array children data
			for (Element& e : arrayData.m_Elements)
				AddArrayElementData(e, nextDataPtr);

			//If the array contains less elements than the specified size then increment the data pointer till the end of the size
			if (arrayData.m_Size.has_value() && *arrayData.m_Size > arrayData.m_Elements.size())
				nextDataPtr += arrayData.m_Elements.back().GetSizeInBytes() * (*arrayData.m_Size - arrayData.m_Elements.size());
		}
		else
		{
			//TODO: Optimize so that data size is passed in not searched for every time
			uint32_t dataSize = 0;
			byte* data = element.m_Data->as<Data::Value&>().m_Data;

			switch (element.m_Type)
			{
			#define X(el) case Type::el: dataSize = Map<Type::el>::SizeBytes(data); break;
				ELEMENT_TYPES
			#undef X
			}

			memcpy(nextDataPtr, data, dataSize);
			nextDataPtr += dataSize;
		}
	}

	Meta::Element& Proton::Asset::AddMetaElement(Element& element, Meta::Addable& addable, std::optional<uint32_t> dataOffset)
	{
		Meta::Element& metaElement = addable.Add(element.m_Name, element.m_Type, dataOffset);
		
		//If no data offset is given then the value is part of a type template, so it will
		//not be given a size
		if(dataOffset.has_value())
			metaElement.m_SizeBytes = element.GetSizeInBytes();

		//Adding a struct to an array template
		if (!dataOffset.has_value() && element.m_Type == Type::Struct)
		{
			Data::Struct& structData = element.m_Data->as<Data::Struct&>();
			//Adds the struct elements to the type template
			for (Element& e : structData.m_Elements)
				AddMetaElement(e, metaElement);
		}
		else if (element.m_Type == Type::Array)
		{
			Data::Array& arrayData = element.m_Data->as<Data::Array&>();
			//Sets the type template of the meta element
			SetMetaArrayTemplate(*element.m_Data, metaElement);
			metaElement.SetSize(arrayData.m_Elements.size());
			
			if (!arrayData.m_constElementSize)
			{
				arrayData.CalcElementOffsets();

				metaElement.SetElementOffsets(arrayData.m_ElementOffsets.value());
				metaElement.m_ExtraData->as<Meta::ExtraData::Array&>().m_ChildStride = arrayData.m_ChildStride;
			}
		}

		return metaElement;
	}

	void Asset::SetMetaArrayTemplate(TypeElement::DataBase& extraData, Meta::Element& metaElement)
	{
		Data::Array& arrayData = static_cast<Data::Array&>(extraData);
		TypeElement& typeTemplate = arrayData.m_TypeTemplate.value();
		metaElement.SetType(typeTemplate.m_Type);

		if (typeTemplate.m_Type == Type::Struct)
		{
			Data::Struct& structData = typeTemplate.m_Data->as<Data::Struct&>();
			//Adds the struct elements to the type template
			for (Element& e : structData.m_Elements)
				AddMetaElement(e, metaElement.GetType());
		}
		else if (typeTemplate.m_Type == Type::Array)
		{
			//Sets the type template of this array
			SetMetaArrayTemplate(*typeTemplate.m_Data, metaElement.GetType());
			//A size of 0 means no size is set
			metaElement.GetType().SetSize(typeTemplate.m_Data->as<Data::Array&>().m_Size.value_or(0));
		}
	}

	ElementRef ElementRef::operator[](const std::string& name)
	{
		Meta::Element& metaElement = m_MetaElement[name];
		return ElementRef(m_Data + (metaElement.m_DataOffset - m_MetaElement.m_DataOffset), metaElement.m_Type, metaElement);
	}

	ElementRef ElementRef::operator[](const char* name)
	{
		return (*this)[std::string(name)];
	}

	ElementRef ElementRef::operator[](int index)
	{
		assert("Element has to be an array" && m_Type == Type::Array);
		Meta::ExtraData::Array& arrayData = m_MetaElement.m_ExtraData->as< Meta::ExtraData::Array&>();

		assert("Index out of bounds" && index < arrayData.m_Size);

		if (arrayData.m_constElementSize)
		{
			Meta::Element metaElement = Meta::Element(arrayData.m_TypeTemplate);
			metaElement.m_SizeBytes = (m_MetaElement.GetSizeInBytes() / arrayData.m_Size);
			if (metaElement.m_Type == Type::Struct)
			{
				Meta::ExtraData::Struct& elementData = metaElement.m_ExtraData->as<Meta::ExtraData::Struct&>();
				metaElement.m_DataOffset = m_MetaElement.m_DataOffset + (index * metaElement.m_SizeBytes);
				int nextOffset = metaElement.m_DataOffset;
				for (Meta::Element& element : elementData.m_Elements)
				{
					element.m_DataOffset = nextOffset;

					if (element.m_Type == Type::Struct)
					{
						element.m_SizeBytes = element.m_ExtraData->as<Meta::ExtraData::Struct&>().CalcStructSize(nextOffset);
					}
					else
					{
						switch (element.m_Type)
						{
							//Null ptr used as strings are not going to be accepted
#define X(el) case Type::el: \
						element.m_SizeBytes = Map<Type::el>::SizeBytes(nullptr); \
						break;
							ELEMENT_TYPES
#undef X
						}
					}
					
					nextOffset += element.m_SizeBytes;
				}
				//int 
			}

			return ElementRef(m_Data + metaElement.m_SizeBytes * index, arrayData.m_TypeTemplate.m_Type, metaElement);
		}
		else
		{
			Meta::Element metaElement = Meta::Element(arrayData.m_TypeTemplate);
			byte* data;
			std::vector<uint32_t>& elementOffsets = arrayData.m_ElementOffsets.value();

			//Note: Element offsets for structs are the offsets of every element in the struct
			if (arrayData.m_TypeTemplate.m_Type == Type::Struct)
			{
				metaElement.m_ExtraData = new Meta::ExtraData::Struct();
				Meta::ExtraData::Struct& elementData = metaElement.m_ExtraData->as<Meta::ExtraData::Struct&>();
				Meta::ExtraData::Struct& typeData = arrayData.m_TypeTemplate.m_ExtraData->as<Meta::ExtraData::Struct&>();
		
				//Base offset of the element offsets
				uint32_t base = typeData.m_Elements.size() * index;
				//Base data offsets
				uint32_t baseOffset = elementOffsets[base];
				//Base data pointer
				data = m_Data + baseOffset;

				//Size is the difference between two element offsets
				metaElement.m_SizeBytes = elementOffsets[base + typeData.m_Elements.size()] - elementOffsets[base];
				metaElement.m_DataOffset = baseOffset + m_MetaElement.m_DataOffset;

				for (int i = 0; i < typeData.m_Elements.size(); i++)
				{
					Meta::Element& el = typeData.m_Elements[i];
					elementData.Add(el.m_Name, el.m_Type, elementOffsets[base + i] + m_MetaElement.m_DataOffset);
				}
			}
			else
			{
				//Size is the difference between two element offsets
				metaElement.m_SizeBytes = index == arrayData.m_Size - 1 ? m_MetaElement.GetSizeInBytes() - elementOffsets[index] : elementOffsets[index + 1] - elementOffsets[index];
				data = m_Data + elementOffsets[index];

				//Calculates the size of the array using the child stride
				if (metaElement.m_Type == Type::Array)
					metaElement.m_ExtraData->as<Meta::ExtraData::Array&>().m_Size = metaElement.m_SizeBytes / arrayData.m_ChildStride;

			}

			return ElementRef(data, arrayData.m_TypeTemplate.m_Type, metaElement);
		}		
	}

	ElementRef Asset::operator[](const std::string& name)
	{
		Meta::Element& metaElement = m_MetaFile[name];
		return ElementRef(m_Data + metaElement.m_DataOffset, metaElement.m_Type, metaElement);
	}

	void AssetSerializer::SerializeAsset(const std::string& filepath, Asset& asset)
	{
		Meta::MetaFileSerializer::SerializeMetaFile(filepath + ".meta", asset.m_MetaFile);

		std::ofstream outStream(filepath, std::ios::out | std::ios::binary);

		outStream.write((const char*)asset.m_Data, asset.m_DataSize);
		outStream.close();
	}

	Asset AssetSerializer::DeserializeAsset(const std::string& filepath)
	{
		Asset asset = *new Asset();
		asset.m_MetaFile = Meta::MetaFileSerializer::DeserializeMetaFile(filepath + ".meta");

		//Reads the file
		std::ifstream inStream(filepath, std::ios::out | std::ios::binary);

		if (!inStream)
		{
			PT_CORE_ERROR("Cannot open file {0}!", filepath);
		}

		std::filebuf* fileBuf = inStream.rdbuf();
		asset.m_DataSize = fileBuf->pubseekoff(0, inStream.end, inStream.in);
		fileBuf->pubseekpos(0, inStream.in);

		asset.m_Data = new byte[asset.m_DataSize];

		fileBuf->sgetn((char*)asset.m_Data, asset.m_DataSize);

		inStream.close();
		//End read file

		//Display Bytes
		/*std::string byteStr = "";
		int lastMod = 1;
		for (int i = 0; i < asset.m_DataSize; i++)
		{
			byte b = asset.m_Data[i];
			byteStr += std::to_string(b) + ' ';
			if (byteStr.length() % 50 < lastMod)
				byteStr += '\n';

			lastMod = byteStr.length() % 50;
		}
		byteStr += "\n\n";

		byteStr = "\n Asset byte data: \n" + byteStr;

		PT_CORE_TRACE(byteStr);*/

		return asset;
	}
}
