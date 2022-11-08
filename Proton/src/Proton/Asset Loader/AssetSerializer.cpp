#include "ptpch.h"
#include "AssetSerializer.h"
#include "MetaFileSerializer.cpp"

namespace Proton
{
	//Extra data for RawAsset
	struct Data
	{
		struct Value : public TypeElement::DataBase
		{
			Value() = default;
			Value(byte* data, Type type)
				:
				m_Data(data)
			{
				uint32_t size;
				switch (type)
				{
					#define X(el) case Type::el: size = Map<Type::el>::SizeBytes(data); break;
					ELEMENT_TYPES
					#undef X
				default:
					assert("Invalid type!");
				}

				m_Data = new byte[size];
				memcpy(m_Data, data, size);
			}

			byte* m_Data = nullptr;
		};

		struct Struct : public TypeElement::DataBase
		{
			bool Contains(const std::string& name)
			{
				for (Element& e : m_Elements)
				{
					if (e.m_Name == name)
						return true;
				}

				return false;
			}

			//Returns true if it contains a string or array
			bool ContainsNonConsistentElement()
			{
				for (Element& e : m_Elements)
				{
					if (e.m_Type == Type::String || e.m_Type == Type::Array)
						return true;

					if (e.m_Type == Type::Struct)
					{
						Data::Struct& structData = e.m_Data->as<Data::Struct&>();
						if (structData.ContainsNonConsistentElement())
							return true;
					}
				}

				return false;
			}

			std::vector<Element> m_Elements;
		};

		struct Array : public TypeElement::DataBase
		{
			void Add(Element& e)
			{
				if (m_TypeTemplate->m_Type == Type::Array)
				{
					Data::Array& typeData = m_TypeTemplate->m_Data->as<Data::Array&>();

					//First element to be added to array
					if (m_Elements.empty() && !typeData.m_Size.has_value())
						m_constElementSize = false;

					Data::Array& arrayData = e.m_Data->as<Data::Array&>();

					arrayData.m_CanBeNonConst = false;
					arrayData.m_Size = typeData.m_Size;
					arrayData.m_constElementSize = typeData.m_constElementSize;
					arrayData.m_TypeTemplate = typeData.m_TypeTemplate;
				}

				//assert("Attempt to add more elements than capacity");

				m_Elements.push_back(e);
			}

			void CalcElementOffsets()
			{
				if (!m_constElementSize)
				{
					m_ElementOffsets = std::vector<uint32_t>({ 0 });
					uint32_t lastOffset = 0;

					for (auto it = m_Elements.begin(); it != (m_Elements.end() - 1); it++)
					{
						lastOffset += it->GetSizeInBytes();
						m_ElementOffsets->push_back(lastOffset);
					}

					if (m_TypeTemplate->m_Type == Type::Array)
						m_ChildStride = m_Elements[0].m_Data->as<Data::Array&>().m_Elements[0].GetSizeInBytes();
				}
			}

			std::optional<TypeElement> m_TypeTemplate;
			std::vector<Element> m_Elements;
			//If the size of the array type is not consistent (ie string type or struct containing string)
			//the array will be converted into a struct when converted to an asset
			bool m_constElementSize;
			std::optional<std::vector<uint32_t>> m_ElementOffsets;

			//Used when type template is array and size needs to be set
			//When it has no value it is assumed that no size restrictions are set
			std::optional<uint32_t> m_Size;

			//If false this array cannot have a non-const type template 
			//(used when this array is a type template for another array)
			bool m_CanBeNonConst = true;

			//The size in bytes of the element of the child array (only used by non-const type arrays with array type templates)
			uint32_t m_ChildStride;
		};
	};

	TypeElement::TypeElement(std::initializer_list<Element> elements)
		:
		m_Type(Type::Struct),
		m_Data(new Data::Struct())
	{
		std::vector<Element>& el = static_cast<Data::Struct&>(*m_Data).m_Elements;
		el = elements;
	}

	TypeElement::TypeElement(Type type)
		:
		m_Type(type),
		m_Data(new Data::Value())
	{
		if (type == Type::Struct)
		{
			m_Data = new Data::Struct();
		}
		else if (type == Type::Array)
		{
			m_Data = new Data::Array();
		}
	}

	TypeElement::TypeElement(Type type, byte* data)
		:
		m_Type(type),
		m_Data(new Data::Value(data, type))
	{
	}

	void TypeElement::Add(TypeElement entry)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		Data::Array& arrayData = static_cast<Data::Array&>(*m_Data);
		assert("Array does not have type template!" && arrayData.m_TypeTemplate.has_value());
		assert("Element does not match array template!" && entry == arrayData.m_TypeTemplate.value());
		if (arrayData.m_Size.has_value())
			assert("Array element size exceeded!" && arrayData.m_Elements.size() < arrayData.m_Size.value());

		Element e = entry;
		int nameSize = 1;
		uint32_t elementSize = arrayData.m_Elements.size();
		if (elementSize > 0xFF)
			nameSize = 2;
		else if (elementSize > 0xFFFF)
			nameSize = 3;
		else if (elementSize > 0xFFFFFF)
			nameSize = 4;

		int indexPlus1 = elementSize + 1;

		e.m_Name = new char[nameSize + 1];
		const_cast<char*>(e.m_Name)[nameSize] = '\0';
		memcpy(const_cast<char*>(e.m_Name), &indexPlus1, nameSize);
		arrayData.Add(std::move(e));
	}

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
				data.m_constElementSize = !structData.ContainsNonConsistentElement();
			}
			else if (type.m_Type == Type::Array)
			{
				Data::Array& arrayData = type.m_Data->as<Data::Array&>();
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

	TypeElement& Proton::TypeElement::GetType()
	{
		return static_cast<Data::Array&>(*m_Data).m_TypeTemplate.value();
	}

	TypeElement& Proton::TypeElement::SetSize(uint32_t size)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		Data::Array& data = static_cast<Data::Array&>(*m_Data);
		assert("Cannot set array size after elements are added!" && data.m_Elements.empty());
		assert("Size must be set before assigning array type template!" && !(data.m_TypeTemplate.has_value() && data.m_TypeTemplate->m_Type == Type::Array));
		data.m_Size = size;

		return *this;
	}

	uint32_t TypeElement::GetSizeInBytes()
	{
		if (m_Type == Type::Struct)
		{
			uint32_t sizeBytes = 0;
			for (Element& e : m_Data->as<Data::Struct&>().m_Elements)
			{
				sizeBytes += e.GetSizeInBytes();
			}

			return sizeBytes;
		}
		else if (m_Type == Type::Array)
		{
			Data::Array& arrayData = m_Data->as<Data::Array&>();

			if (arrayData.m_constElementSize)
			{
				return arrayData.m_Elements.back().GetSizeInBytes() * arrayData.m_Size.value_or(arrayData.m_Elements.size());
			}
			else
			{
				if (!arrayData.m_ElementOffsets.has_value())
					arrayData.CalcElementOffsets();

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

	/*Meta::Element TypeElement::ToArrayMetaElement()
	{
		Meta::Element element = Meta::Element("", m_Type, );
		if (m_Type == Type::Struct)
		{
			element = Meta::Element("");
			for (Element& e : m_Data->as<Data::Struct&>().m_Elements)
				element.Add(e.ToMetaElement());
		}
		else if (m_Type == Type::Array)
		{
			Data::Array& arrayData = m_Data->as<Data::Array&>();
			element = Meta::Element("", m_Type, arrayData.m_Elements.size());
			element.SetTypeTemplate(arrayData.m_TypeTemplate.value().ToArrayMetaElement());
		}
		else
		{
			element = Meta::Element("", m_Type);
		}

		return element;
	}*/

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

	/*Element::Element(const char* name, std::initializer_list<Element> initialiser)
		:
		TypeElement(Type::Struct),
		m_Name(name)
	{
		assert("Illegal character \'|\' used!" && !strchr(name, '|'));
		Data::Struct& structData = static_cast<Data::Struct&>(*m_Data);
		structData.m_Elements = initialiser;
	}

	/*Element::Element(const char* name, TypeElement typeTemplate, std::initializer_list<TypeElement> initialiser)
		:
		TypeElement(Type::Array),
		m_Name(name)
	{
		assert("Illegal character \'|\' used!" && !strchr(name, '|'));
		//assert("Cannot create an array of strings!" && typeTemplate.m_Type != Type::String);

		for (const TypeElement& e : initialiser)
		{
			assert("Item in initialiser list does not match the type template provided!" && e == typeTemplate);
		}

		Data::Array& arrayData = static_cast<Data::Array&>(*m_Data);
		arrayData.m_Elements = initialiser;

		//Check if array contains type template whose size is not consistent
		arrayData.m_constElementSize = typeTemplate.m_Type == Type::String || typeTemplate.m_Type == Type::Array;
		if (!arrayData.m_constElementSize)
		{
			if (typeTemplate.m_Type == Type::Struct)
			{
				Data::Struct& structData = typeTemplate.m_Data->as<Data::Struct&>();
				arrayData.m_constElementSize = structData.ContainsNonConsistentElement();
			}
		}

		arrayData.m_TypeTemplate = std::move(typeTemplate);
	}*/

	Element::Element(const char* name, Type type, byte* data)
		:
		TypeElement(type),
		m_Name(name)
	{
		assert("Illegal character \'|\' used!" && !strchr(name, '|'));
		m_Data = new Data::Value(data, type);
	}

	Element::Element(const char* name, Type type)
		:
		TypeElement(type),
		m_Name(name)
	{
		assert("Illegal character \'|\' used!" && !strchr(name, '|'));
	}

	void Element::Add(Element entry)
	{
		assert("Element is not a struct or array!" && (m_Type == Type::Struct || m_Type == Type::Array));
		if (m_Type == Type::Struct)
		{
			Data::Struct& structData = static_cast<Data::Struct&>(*m_Data);
			assert("Element with same name already exists!" && !structData.Contains(entry.m_Name));
			structData.m_Elements.push_back(std::move(entry));
		}
		else
		{
			((TypeElement*)this)->Add(static_cast<TypeElement>(entry));
		}
	}

	void Element::Add(TypeElement element)
	{
		((TypeElement*)this)->Add(static_cast<TypeElement>(element));
	}

	Element& TypeElement::operator[](const std::string& name)
	{
		assert("Element is not a struct!" && m_Type == Type::Struct);
		Data::Struct& structData = static_cast<Data::Struct&>(*m_Data);

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
		Data::Array& data = static_cast<Data::Array&>(*m_Data);
		assert("Index out of bounds" && index < data.m_Elements.size());

		return data.m_Elements[index];
	}

	//Asset

	/*ElementData::ElementData(const std::string& name, Type type, byte* dataPtr, TypeElement::DataBase* elementData, std::string structKey)
		:
		m_Name(name),
		m_Type(type),
		m_Data(dataPtr)
	{
		if (type == Type::Struct)
		{
			assert("No struct key was given!" && !structKey.empty());
			m_ExtraData = new Struct(structKey);
		}
		else if (type == Type::Array)
		{
			Data::Array& arrayData = elementData->as<Data::Array&>();
			assert("No struct key was given!" && !(structKey.empty() && !arrayData.m_constElementSize));
			m_ExtraData = new Array(arrayData.m_TypeTemplate.value(), arrayData.m_Elements.size(), arrayData.m_constElementSize, std::move(!arrayData.m_constElementSize && !structKey.empty() ? std::optional<std::string>(structKey) : std::nullopt));
		}
	}*/

	Asset::Asset(RawAsset rawAsset)
		:
		m_MetaFile(rawAsset.m_Elements.size())
	{
		uint32_t numElements = 0;

		for (Element& e : rawAsset.m_Elements)
		{
			GetNumElementsAndSize(e, numElements, m_DataSize);
		}

		m_Data = new byte[m_DataSize];
		byte* nextData = m_Data;

		std::string nextStructTag = std::string("\001");

		for (Element& e : rawAsset.m_Elements)
		{
			AddElement(e, m_Data, nextData, m_MetaFile, nextStructTag);
			//e.AddElementToMetaFile((uint32_t)(nextData - m_Data), m_MetaFile);
		}
	}

	void Asset::GetNumElementsAndSize(Element& element, uint32_t& numElements, uint32_t& size)
	{
		numElements++;

		if (element.m_Type != Type::Struct)
		{
			if (element.m_Type == Type::Array)
			{
				Data::Array& arrayData = element.m_Data->as<Data::Array&>();
				if (arrayData.m_constElementSize) 
				{
					for (TypeElement& e : arrayData.m_Elements)
					{
						GetNumElementsAndSize(e, numElements, size);
					}
					return;
				}
			}
			size += element.GetSizeInBytes();
		}
		else
		{
			Data::Struct& structData = element.m_Data->as<Data::Struct&>();
			for (Element& e : structData.m_Elements)
			{
				GetNumElementsAndSize(e, numElements, size);
			}
		}
	}

	void Proton::Asset::AddArrayElementData(Element& element, byte*& nextDataPtr)
	{
		assert("Array data cannot be of variable size!" && !(element.m_Type == Type::Array && !element.m_Data->as<Data::Array>().m_constElementSize));
		if (element.m_Type == Type::Struct)
		{
			for (Element& e : element.m_Data->as<Data::Struct&>().m_Elements)
			{
				AddArrayElementData(e, nextDataPtr);
			}
		}
		else if (element.m_Type == Type::Array)
		{
			Data::Array& arrayData = element.m_Data->as<Data::Array&>();
			for (Element& e : arrayData.m_Elements)
			{
				AddArrayElementData(e, nextDataPtr);
			}

			if (arrayData.m_Size.has_value() && *arrayData.m_Size > arrayData.m_Elements.size())
			{
				nextDataPtr += arrayData.m_Elements.back().GetSizeInBytes() * (*arrayData.m_Size - arrayData.m_Elements.size());
			}
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

	//~ is the symbol for an array struct
	void Asset::AddElement(Element& element, const byte* dataPtr, byte*& nextDataPtr, Meta::Addable& addable, std::string& nextStructTag, std::string structTag)
	{
		Meta::Element& metaElement = AddMetaElement(element, addable, (uint32_t)(nextDataPtr - dataPtr));
		
		if (element.m_Type == Type::Struct)
		{
			for (Element& e : element.m_Data->as<Data::Struct&>().m_Elements)
			{
				AddElement(e, dataPtr, nextDataPtr, metaElement, nextStructTag, structTag);
			}
		}
		else if (element.m_Type == Type::Array)
		{
			Data::Array& arrayData = element.m_Data->as<Data::Array&>();

			for (Element& e : arrayData.m_Elements)
			{
				AddArrayElementData(e, nextDataPtr);
			}

			if (arrayData.m_Size.has_value() && *arrayData.m_Size > arrayData.m_Elements.size())
			{
				nextDataPtr += arrayData.m_Elements.back().GetSizeInBytes() * (*arrayData.m_Size - arrayData.m_Elements.size());
			}
		}
		else
		{
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

		if (element.m_Type == Type::Array)
		{
			Data::Array& arrayData = element.m_Data->as<Data::Array&>();
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
			for (Element& e : structData.m_Elements)
			{
				AddMetaElement(e, metaElement.GetType());
			}
		}
		else if (typeTemplate.m_Type == Type::Array)
		{
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

			return ElementRef(m_Data + metaElement.m_SizeBytes * index, arrayData.m_TypeTemplate.m_Type, metaElement);
		}
		else
		{
			Meta::Element metaElement = Meta::Element(arrayData.m_TypeTemplate);
			metaElement.m_SizeBytes = index == arrayData.m_Size - 1 ? m_MetaElement.GetSizeInBytes() - arrayData.m_ElementOffsets.value()[index] : arrayData.m_ElementOffsets.value()[index + 1] - arrayData.m_ElementOffsets.value()[index];
			
			if (metaElement.m_Type == Type::Array)
			{
				metaElement.m_ExtraData->as<Meta::ExtraData::Array&>().m_Size = metaElement.m_SizeBytes / arrayData.m_ChildStride;
			}

			return ElementRef(m_Data + arrayData.m_ElementOffsets.value()[index], arrayData.m_TypeTemplate.m_Type, metaElement);
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

		//Display Bytes
		std::string byteStr = "";
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

		PT_CORE_TRACE(byteStr);

		return asset;
	}

	void GetNumElements(Meta::Element& element, uint32_t& numElements)
	{
		numElements++;
		if (element.m_Type == Type::Struct)
		{
			for (Meta::Element& e : element.m_ExtraData->as<Meta::ExtraData::Struct&>().m_Elements)
				GetNumElements(e, numElements);
		}
		else if (element.m_Type == Type::Array)
		{
			Meta::ExtraData::Array& arrayData = element.m_ExtraData->as<Meta::ExtraData::Array&>();
			if (!arrayData.m_constElementSize)
			{
				for (Meta::Element& e : element.m_ExtraData->as<Meta::ExtraData::Struct&>().m_Elements)
					GetNumElements(e, numElements);
			}
		}
	}
}