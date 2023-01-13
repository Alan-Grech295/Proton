#pragma once
#include <string>
#include <vector>
#include "Proton\Core\Log.h"
#include <unordered_map>
#include <assert.h>
#include "MetaFileSerializer.h"
#include <optional>

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
		void Add(TypeElement entry);
		template<typename T>
		void Add(T&& data);

		//Array set type function
		TypeElement& SetType(TypeElement type);

		//Array get type function (cannot set type from here)
		TypeElement& GetType();

		//Array set size function
		TypeElement& SetSize(uint32_t size);

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

		//TODO: Create Add function templates

		//Struct Add function
		template<typename T>
		void Add(const char* name, T&& data);

		//Array Add function
		void Add(TypeElement entry);
		template<typename T>
		void Add(T&& data)
		{
			((TypeElement*)this)->Add(std::forward<T>(data));
		}

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

	template<Type type>
	struct Map
	{
		static constexpr bool valid = false;
	};
	template<> struct Map<Type::Byte>
	{
		using SysType = byte; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return &data;
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return sizeof(SysType);
		}
	};
	template<> struct Map<Type::Int16>
	{
		using SysType = int16_t; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return (byte*)&data;
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return sizeof(SysType);
		}
	};
	template<> struct Map<Type::Int32>
	{
		using SysType = int32_t; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return (byte*)&data;
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return sizeof(SysType);
		}
	};
	template<> struct Map<Type::Int64>
	{
		using SysType = int64_t; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return (byte*)&data;
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return sizeof(SysType);
		}
	};
	template<> struct Map<Type::Float>
	{
		using SysType = float; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return (byte*)&data;
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return sizeof(SysType);
		}
	};
	template<> struct Map<Type::Double>
	{
		using SysType = double; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return (byte*)&data;
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return sizeof(SysType);
		}
	};
	template<> struct Map<Type::String>
	{
		using SysType = std::string; // type used in the CPU side
		static constexpr bool valid = true; // metaprogramming flag to check validity of Map <param>

		static byte* GetData(SysType& data)
		{
			return (byte*)data.c_str();
		}

		static uint32_t SizeBytes(const byte* data)
		{
			return strlen((const char*)data) + 1;
		}
	};

	// ensures that every leaf type in master list has an entry in the static attribute map
	#define X(el) static_assert(Map<Type::el>::valid,"Missing map implementation for " #el);
	ELEMENT_TYPES
	#undef X

	// enables reverse lookup from SysType to leaf type
	template<typename T>
	struct ReverseMap
	{
		static constexpr bool valid = false;
	};
	#define X(el) \
	template<> struct ReverseMap<typename Map<Type::el>::SysType> \
	{ \
		static constexpr Type type = Type::el; \
		static constexpr bool valid = true; \
	};
	ELEMENT_TYPES
	#undef X

	class RawAsset
	{
		friend class Asset;
	public:
		template<typename T>
		void Add(const std::string& name, T&& data)
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
			constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
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

		Element& operator[](const std::string& name)
		{
			return m_Elements[m_ElementLocator[name]];
		}
	private:
		std::vector<Element> m_Elements;
		std::unordered_map<std::string, uint32_t> m_ElementLocator;
	};

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

			//Checks if the struct contains an element of non const size
			bool ConstantSize()
			{
				for (Element& e : m_Elements)
				{
					if (e.m_Type == Type::String || e.m_Type == Type::Array)
						return false;

					if (e.m_Type == Type::Struct)
					{
						Data::Struct& structData = e.m_Data->as<Data::Struct&>();
						if (!structData.ConstantSize())
							return false;
					}
				}

				return true;
			}

			void Add(Element entry)
			{
				assert("Element with same name already exists!" && !Contains(entry.m_Name));
				assert("Cannot add array or struct to type template!" && !(m_IsTypeTemplate && (entry.m_Type == Type::Array || entry.m_Type == Type::Struct)));

				m_Elements.push_back(std::move(entry));
			}

			std::vector<Element> m_Elements;
			bool m_IsTypeTemplate = false;
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

					//Does not allow the type template to be have a non-const type template
					arrayData.m_CanBeNonConst = false;
					arrayData.m_Size = typeData.m_Size;
					arrayData.m_constElementSize = typeData.m_constElementSize;
					arrayData.m_TypeTemplate = typeData.m_TypeTemplate;
				}
				else if (m_TypeTemplate->m_Type == Type::Struct)
				{
					Data::Struct& elementData = e.m_Data->as<Data::Struct&>();

					if (!m_ElementOffsets.has_value())
						m_ElementOffsets = std::vector<uint32_t>({ 0 });

					//Adds the offset of each element in the struct to the element offsets
					for (Element& e : elementData.m_Elements)
						m_ElementOffsets->push_back(m_ElementOffsets->back() + e.GetSizeInBytes());
				}

				m_Elements.push_back(e);
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

					//If the type template is an array the child stride of the template array is calculated
					//Note: The first element size is taken instead of the type template as some data types
					//give incorrect sizes with no data?
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

	struct ExtraData
	{
		virtual ~ExtraData() = default;
	};

	struct ElementRef
	{
	public:
		ElementRef(byte* data, Type type, Meta::Element& metaElement)
			:
			m_Data(data),
			m_Type(type),
			m_MetaElement(metaElement)
		{
			switch (type)
			{
			#define X(el) case Type::el: m_Size = Map<Type::el>::SizeBytes(data); break;
				ELEMENT_TYPES
			#undef X
			default:
				assert("Invalid type!");
			}
		}

		template<typename T>
		operator T&()
		{
			assert("Cannot convert structs or arrays to data type!" && m_Type != Type::Struct && m_Type != Type::Array);
			assert("Element being cast to type of incompatible size!" && sizeof(T) == m_Size);
			return *((T*)m_Data);
		}

		//String conversion operator because the standard T& operator does not work
		operator std::string&()
		{
			char* data = (char*)m_Data;
			uint32_t len = strlen(data);
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

	private:
		byte* m_Data;
		Type m_Type;
		uint32_t m_Size = 0;
		Meta::Element m_MetaElement;
	};

	//Note: Variables cannot be added to Assets, however, data can be changed
	class Asset
	{
		friend class AssetSerializer;
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
	inline void Element::Add(const char* name, T&& data)
	{
		assert("Element is not a struct or array!" && (m_Type == Type::Struct || m_Type == Type::Array));
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		Element entry = Element(name, type, Map<type>::GetData(data));

		if (m_Type == Type::Struct)
			m_Data->as<Data::Struct&>().Add(entry);
		else
			((TypeElement*)this)->Add(static_cast<TypeElement>(entry));
	}

	template<typename T>
	inline void TypeElement::Add(T&& data)
	{
		assert("Element is not an array!" && m_Type == Type::Array);
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		TypeElement entry = TypeElement(type, Map<type>::GetData(data));

		((TypeElement*)this)->Add(static_cast<TypeElement>(entry));
	}

	template<typename T>
	inline Element Element::Create(const char* name, T&& data)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		return Element(name, type, Map<type>::GetData(data));
	}
	
	template<typename T>
	inline TypeElement TypeElement::Create(T&& data)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		return TypeElement(type, Map<type>::GetData(data));
	}
}
