#pragma once
#include "MetaFileSerializer.h"

namespace Proton
{
	//This element only stored the type. It is used for array types
	class TypeElement
	{
	public:
		//Extra data for RawAsset
		struct DataBase
		{
			virtual ~DataBase() = default;
		public:
			template<typename T>
			T& as();
		};

		friend struct Data;
	public:
		TypeElement() = default;

		//TODO: Check struct child names
		//Struct constructor
		TypeElement(std::initializer_list<class Element> elements);

		TypeElement(Type type);

		TypeElement(Type type, byte* data);

		//Array Add function
		uint32_t Add(TypeElement entry);
		template<typename T>
		uint32_t Add(T&& data);

		//Array set type function
		TypeElement& SetType(TypeElement type);

		//Array get type function (cannot set type from here)
		TypeElement& GetType();

		//Array set size function
		TypeElement& SetSize(uint32_t size);

		//Struct Add function
		template<typename T>
		Element& Add(const char* name, T&& data);

		Element& Add(const char* name, Type type);

		//Struct and Array Add
		Element& Add(std::initializer_list<Element> elements);

		//Array Add
		Element& Add(std::initializer_list<TypeElement> elements);

		//Pointer set element type function
		void SetPointer(Type type, class RawAsset& asset);

		template<typename T>
		void SetData(T&& data);

		uint32_t GetSizeInBytes();

		bool operator==(TypeElement other)
		{
			return Equals(other);
		}

		const bool operator==(TypeElement other) const
		{
			return Equals(other);
		}

		bool operator!=(TypeElement other)
		{
			return !Equals(other);
		}

		const bool operator!=(TypeElement other) const
		{
			return !Equals(other);
		}

		//Pointer access
		TypeElement* operator->();

		//Array Search function
		TypeElement& operator[](uint32_t index);

		//Struct Search function
		Element& operator[](const std::string& name);

		operator Element();
		operator const Element() const;
		operator Element&();
		operator const Element&() const;

		template<typename T>
		static TypeElement Create(T&& data);
	private:
		bool Equals(TypeElement& other);

		const bool Equals(TypeElement& other) const;
	public:
		Type m_Type;
		DataBase* m_Data = nullptr;
	};

	//TODO: Check that no variables are named the same
	//TODO: Convert all Element constructors to Element::Create
	class Element : public TypeElement
	{
	public:
		Element() = default;

		Element(const char* name, Type type, byte* data);

		Element(const char* name, Type type);

		Element(const char* name, std::initializer_list<Element> elements);

		//TODO: Create Add function templates

		//Compare operator for type and element
		bool operator==(TypeElement& other)
		{
			return static_cast<TypeElement&>(*this) == other;
		}

		const bool operator==(TypeElement& other) const
		{
			return static_cast<const TypeElement&>(*this) == other;
		}

		bool operator!=(TypeElement& other)
		{
			return !(static_cast<const TypeElement&>(*this) == other);
		}

		const bool operator!=(TypeElement& other) const
		{
			return !(static_cast<const TypeElement&>(*this) == other);
		}

		template<typename T>
		static Element Create(const char* name, T&& data);

		static Element& Empty()
		{
			static Element empty = Element(nullptr, Type::None);
			return empty;
		}
	public:
		const char* m_Name;
	};

	class RawAsset
	{
		friend class Asset;
	public:
		template<typename T>
		void Add(const std::string& name, T&& data)
		{
			static_assert(ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::valid, "Unsupported SysType used in pointer conversion");
			constexpr Type type = ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::type;
			auto [it, succeded] = m_ElementLocator.try_emplace(name, m_Elements.size());
			assert("Element with same name already exists!" && succeded);
			m_Elements.push_back(Element(it->first.c_str(), type, Map<type>::GetData(data)));
		}

		void Add(const std::string& name, Type type)
		{
			auto [it, succeded] = m_ElementLocator.try_emplace(name, m_Elements.size());
			assert("Element with same name already exists!" && succeded);
			m_Elements.push_back(Element(it->first.c_str(), type));
		}

		uint32_t AddPointer(Type type);

		Element& operator[](const std::string& name)
		{
			return m_Elements[m_ElementLocator[name]];
		}
	private:
		std::vector<Element> m_Elements;
		std::unordered_map<std::string, uint32_t> m_ElementLocator;
		uint32_t m_NextPtr = 1;
	};

	//Extra data for RawAsset
	struct Data
	{
		struct Value : public TypeElement::DataBase
		{
			Value() = default;
			~Value() 
			{
				delete m_Data;
			}

			Value(byte* data, Type type)
			{
				uint32_t size = 0;
				//Gets the size of the type depending on a Map
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

			void SetConstElementSize()
			{
				m_OnlyConstElements = true;
				assert("Struct already contains non-const element!" && m_ConstSize);
				for (Element& e : m_Elements)
				{
					if (e.m_Type == Type::Struct)
						e.m_Data->as<Data::Struct&>().SetConstElementSize();

					assert("Struct already contains non-const element!" && !(e.m_Type == Type::String || (e.m_Type == Type::Array && !e.m_Data->as<Data::Array>().m_constElementSize)));
				}
			}

			void Add(Element entry)
			{
				assert("Element with same name already exists!" && !Contains(entry.m_Name));
				assert("Cannot add string or array to type template!" && !(m_OnlyConstElements && (entry.m_Type == Type::String || entry.m_Type == Type::Array)));
								
				m_Elements.push_back(std::move(entry));
				if (entry.m_Type == Type::Struct && m_OnlyConstElements)
				{
					m_Elements.back().m_Data->as<Data::Struct&>().SetConstElementSize();
				}

				if (m_ConstSize && (entry.m_Type == Type::String || (entry.m_Type == Type::Array && !entry.m_Data->as<Data::Array>().m_constElementSize)))
					m_ConstSize = false;
			}

			std::vector<Element> m_Elements;
			bool m_OnlyConstElements = false;
			bool m_ConstSize = true;
		};

		struct Array : public TypeElement::DataBase
		{
			Array()
			{
			}

			void Add(Element e)
			{
				if (m_TypeTemplate->m_Type == Type::Array)
				{
					Data::Array& typeData = m_TypeTemplate->m_Data->as<Data::Array&>();

					//First element to be added to array
					if (m_Elements.empty() && !typeData.m_Size.has_value())
						m_constElementSize = false;

					Data::Array& arrayData = e.m_Data->as<Data::Array&>();

					//Does not allow the type template to be have a non-const type template
					arrayData.m_CanBeNonConst = false;
					arrayData.m_Size = typeData.m_Size;
					arrayData.m_constElementSize = typeData.m_constElementSize;
					arrayData.m_TypeTemplate = typeData.m_TypeTemplate;
				}
				else if (m_TypeTemplate->m_Type == Type::Struct)
				{
					Data::Struct& elementData = e.m_Data->as<Data::Struct>();
					Data::Struct& typeData = m_TypeTemplate->m_Data->as<Data::Struct>();

					if (m_Elements.empty())
						m_constElementSize = typeData.m_ConstSize;

					if (!m_constElementSize)
					{
						if (!m_constElementSize && !m_ElementOffsets.has_value())
							m_ElementOffsets = std::vector<uint32_t>({ 0 });

						//Adds the offset of each element in the struct to the element offsets
						for (Element& e : elementData.m_Elements)
							m_ElementOffsets->push_back(m_ElementOffsets->back() + e.GetSizeInBytes());
					}
				}

				m_Elements.push_back(e);
			}

			void CalcStride()
			{
				if(m_constElementSize)
					m_Stride = m_TypeTemplate->GetSizeInBytes();

				if (m_TypeTemplate->m_Type == Type::Array)
				{
					m_TypeTemplate->m_Data->as<Data::Array>().CalcStride();
				}
			}

			//Creates the element offsets
			void CalcElementOffsets()
			{
				if (!m_constElementSize && m_TypeTemplate->m_Type != Type::Struct)
				{
					m_ElementOffsets = std::vector<uint32_t>({ 0 });
					uint32_t lastOffset = 0;

					//Loops over all elements except the last and adds the element offset
					for (auto it = m_Elements.begin(); it != (m_Elements.end() - 1); it++)
					{
						lastOffset += it->GetSizeInBytes();
						m_ElementOffsets->push_back(lastOffset);
					}
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

			//The size in bytes of the type template
			int m_Stride = -1;
		};

		struct Pointer : public TypeElement::DataBase
		{
			void SetPointer(uint32_t pointer, RawAsset& asset)
			{
				m_Pointer = pointer;
				m_PointerToString = GetTag(m_Pointer);
				m_Asset = &asset;
			}

			static const std::string GetTag(uint32_t index)
			{
				return tag + std::to_string(index);
			}
		public:
			uint32_t m_Pointer = 0;
			std::string m_PointerToString;
			RawAsset* m_Asset = nullptr;
			static const std::string tag;
		};
	};

	struct ExtraData
	{
		virtual ~ExtraData() = default;
	};

	struct ElementRef
	{
	public:
		struct StructIterator
		{
		public:
			StructIterator(std::vector<Meta::Element>::iterator iterator, uint32_t size, byte* baseData, class Asset& asset)
				:
				m_BaseData(baseData),
				m_Asset(asset),
				m_ElementIter(iterator),
				m_Index(0),
				m_Size(size)
			{}

			ElementRef& operator*();

			ElementRef* operator->();

			StructIterator& operator++();

			StructIterator operator++(int);

			StructIterator& operator--();

			StructIterator operator--(int);

			bool operator==(const StructIterator& other) const;

			bool operator!=(const StructIterator& other) const;
		private:
			std::vector<Meta::Element>::iterator m_ElementIter;
			uint32_t m_Index;
			byte* m_BaseData;
			class Asset& m_Asset;
			Ref<std::vector<ElementRef>> m_ElementRefs;
			
			uint32_t m_Size;
		};

	public:
		using struct_iterator = StructIterator;
	public:
		ElementRef(byte* data, Type type, Asset& asset, Meta::Element& metaElement)
			:
			m_Data(data),
			m_Type(type),
			m_MetaElement(metaElement),
			m_Asset(asset)
		{
			m_Size = m_MetaElement.m_SizeBytes;
		}

		ElementRef operator*();

		template<typename T>
		operator T&()
		{
			//assert("Cannot convert structs or arrays to data type!" && m_Type != Type::Struct && m_Type != Type::Array);
			assert("Element being cast to type of incompatible size!" && sizeof(T) == m_Size);
			return *((T*)m_Data);
		}

		//String conversion operator because the standard T& operator does not work
		operator std::string&()
		{
			char* data = (char*)m_Data;
			uint32_t len = (uint32_t)strlen(data);
			return *(new std::string(data, len));
		}

		//Char pointer conversion operator because the standard T& operator does not work
		operator char*()
		{
			return (char*)m_Data;
		}

		//Accesses element in a struct
		ElementRef operator[](const std::string& name);
		ElementRef operator[](const char* name);

		//Accesses element in an array
		ElementRef operator[](int index);

		bool Has(const std::string& name);

		bool operator==(ElementRef& other) const { return m_Data == other.m_Data; }
		bool operator!=(ElementRef& other) const { return m_Data != other.m_Data; }
		//static bool operator==(ElementRef& a, ElementRef& b) { return a.m_Data == b.m_Data; }
		//static bool operator!=(ElementRef& a, ElementRef& b) { return a.m_Data != b.m_Data; }

		uint32_t Size();

		//Convert to struct
		struct StructRef AsStruct();
	public:
		byte* m_Data;
		Type m_Type;
		uint32_t m_Size = 0;
		Asset& m_Asset;
		Meta::Element m_MetaElement;

		static ElementRef& NULL_ELEMENT;

		//For struct use
		//std::tuple<std::string, uint32_t> m_CachedElement;
	};

	struct StructRef : public ElementRef
	{
	public:
		StructRef(const ElementRef& element)
			:
			ElementRef(element)
		{
			assert("Element not a struct" && element.m_Type == Type::Struct);
		}

		StructIterator begin();
		StructIterator end();
	};

	//Note: Variables cannot be added to Assets, however, data can be changed
	class Asset
	{
		friend class AssetSerializer;
		friend struct StructRef;
	public:
		Asset()
			:
			m_Data(nullptr),
			m_DataSize(0),
			m_MetaFile(0)
		{}

		//Creates an asset from a raw asset
		Asset(RawAsset rawAsset);

		//Accesses a variable by name
		ElementRef operator[](const std::string& name);
	private:
		//Gets the number of element and the size of the file to store
		//void GetNumElementsAndSize(Element& element, uint32_t& numElements, uint32_t& size);

		//Adds an element to the asset (element data to m_Data and element info to m_MetaFile)
		//Used when converting a raw asset to an asset
		void AddElement(Element& element, const byte* dataPtr, byte*& nextDataPtr, Meta::Addable& addable);

		//Adds a meta element corresponding to an element to the meta file
		Meta::Element& AddMetaElement(Element& element, Meta::Addable& addable, std::optional<uint32_t> dataOffset = std::nullopt);

		//Adds the element data when it is in an array
		void AddArrayElementData(Element& element, byte*& nextDataPtr);
	
		//Sets the type template of the meta element based on the element template
		void SetMetaArrayTemplate(TypeElement::DataBase& arrayData, Meta::Element& metaElement);
	private:
		byte* m_Data;
		uint32_t m_DataSize = 0;
		Meta::MetaFile m_MetaFile;
	};

	class AssetSerializer
	{
	public:
		static void SerializeAsset(const std::string& filepath, Asset& asset);

		static Asset DeserializeAsset(const std::string& filepath);
	};

	template<typename T>
	inline Element& TypeElement::Add(const char* name, T&& data)
	{
		assert("Element is not a struct or array!" && (m_Type == Type::Struct || m_Type == Type::Array));
		static_assert(ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::type;
		Element entry = Element(name, type, Map<type>::GetData(data));

		if (m_Type == Type::Struct)
			m_Data->as<Data::Struct&>().Add(entry);
		else
			((TypeElement*)this)->Add(static_cast<TypeElement>(entry));

		return *this;
	}

	template<typename T>
	inline uint32_t TypeElement::Add(T&& data)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		static_assert(ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::type;
		TypeElement entry = TypeElement(type, Map<type>::GetData(data));

		((TypeElement*)this)->Add(static_cast<TypeElement>(entry));
		return (uint32_t)m_Data->as<Data::Array&>().m_Elements.size() - 1;
	}

	template<typename T>
	inline Element Element::Create(const char* name, T&& data)
	{
		static_assert(!std::is_same<T, Type>::value, "Type passed into Element::Create. Used Element constructor instead");
		static_assert(ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::type;
		return Element(name, type, Map<type>::GetData(data));
	}

	template<typename T>
	inline void TypeElement::SetData(T&& data)
	{
		static_assert(ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::type;
		assert("Data type does not match element type" && type == m_Type);
		delete m_Data;
		m_Data = new Data::Value(Map<type>::GetData(data), type);
	}
	
	template<typename T>
	inline TypeElement TypeElement::Create(T&& data)
	{
		static_assert(ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<std::remove_reference_t<T>>>::type;
		return TypeElement(type, Map<type>::GetData(data));
	}
}
