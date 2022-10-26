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

		//TODO: Arrays of arrays
		//TODO: Check struct variable names so there is no overlapping
		//Struct constructor
		TypeElement(std::initializer_list<class Element> elements);

		TypeElement(Type type);

		TypeElement(Type type, byte* data);

		//Array Add function
		void Add(TypeElement entry);

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
		static TypeElement Create(T& data);

		template<typename T>
		static TypeElement Create(T&& data);

		//Convert element to MetaFile element
		/*Meta::Element ToArrayMetaElement();*/

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

		/*//Struct constructor
		Element(const char* name, std::initializer_list<Element> initialiser);

		//Array constructor
		Element(const char* name, TypeElement typeTemplate, std::initializer_list<TypeElement> initialiser);*/

		Element(const char* name, Type type, byte* data);

		Element(const char* name, Type type);

		//TODO: Create Add function templates

		//Struct Add function
		void Add(Element element);

		//Array Add function
		void Add(TypeElement element);

		//Convert element to MetaFile element
		void AddElementToMetaFile(uint32_t dataOffset, Meta::Addable& addable);

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
		static Element Create(const char* name, T& data);

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
		void Add(const std::string& name, T& data)
		{
			static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
			constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
			auto [it, succeded] = m_ElementLocator.try_emplace(name, m_Elements.size());
			assert("Element with same name already exists!" && succeded);
			m_Elements.push_back(Element(it->first.c_str(), type, Map<type>::GetData(data)));
		}

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

		//Struct add function
		/*void Add(const std::string& name, std::initializer_list<Element> elements)
		{
			auto [it, succeded] = m_ElementLocator.try_emplace(name, m_Elements.size());
			assert("Element with same name already exists!" && succeded);
			m_Elements.push_back(Element(it->first.c_str(), std::move(elements)));
		}

		//Array add function
		void Add(const std::string& name, TypeElement typeElement, std::initializer_list<TypeElement> elements)
		{
			auto [it, succeded] = m_ElementLocator.try_emplace(name, m_Elements.size());
			assert("Element with same name already exists!" && succeded);
			m_Elements.push_back(Element(it->first.c_str(), std::move(typeElement), std::move(elements)));
		}*/

		Element& operator[](const std::string& name)
		{
			return m_Elements[m_ElementLocator[name]];
		}
	private:
		std::vector<Element> m_Elements;
		std::unordered_map<std::string, uint32_t> m_ElementLocator;
	};

	struct ExtraData
	{
		virtual ~ExtraData() = default;
	};

	struct ElementRef
	{
	public:
		ElementRef(byte* data, Type type)
			:
			m_Data(data),
			m_Type(type)
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
			if (m_Type != Type::String)
				assert("Element being cast to type of incompatible size!" && sizeof(T) == m_Size);
			else
				assert("Element being cast to type of incompatible size!" && std::is_same<T, std::string>::value || std::is_same<T, char*>::value);
		
			return *((T*)m_Data);
		}

	private:
		byte* m_Data;
		Type m_Type;
		uint32_t m_Size;
	};

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

		Asset(RawAsset rawAsset);

	private:
		void GetNumElementsAndSize(Element& element, uint32_t& numElements, uint32_t& size);

		void AddElement(Element& element, const byte* dataPtr, byte*& nextDataPtr, Meta::Addable& addable, std::string& nextStructTag, std::string structTag = "");

		Meta::Element& AddMetaElement(Element& element, Meta::Addable& addable, std::optional<uint32_t> dataOffset = std::nullopt);

		void AddArrayElementData(Element& element, byte*& nextDataPtr);
	
		void SetMetaArrayTemplate(TypeElement::DataBase& arrayData, Meta::Element& metaElement);
	private:
		//std::unordered_map<std::string, ElementData*> m_ElementLocator;
		//ElementData* m_ElementData;
		byte* m_Data;
		uint32_t m_DataSize = 0;
		Meta::MetaFile m_MetaFile;
	};

	class AssetSerializer
	{
	public:
		static void SerializeAsset(const std::string& filepath, Asset& asset);

		static Asset DeserializeAsset(const std::string& filepath);

	private:
		static void GetNumElements(Meta::Element& element, uint32_t& numElements);
	};

	template<typename T>
	inline Element Element::Create(const char* name, T& data)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		return Element(name, type, Map<type>::GetData(data));
	}
	template<typename T>
	inline Element Element::Create(const char* name, T&& data)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		return Element(name, type, Map<type>::GetData(data));
	}
	template<typename T>
	inline TypeElement TypeElement::Create(T& data)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		return TypeElement(type, Map<type>::GetData(data));
	}
	template<typename T>
	inline TypeElement TypeElement::Create(T&& data)
	{
		static_assert(ReverseMap<std::remove_const_t<T>>::valid, "Unsupported SysType used in pointer conversion");
		constexpr Type type = ReverseMap<std::remove_const_t<T>>::type;
		return TypeElement(type, Map<type>::GetData(data));
	}
}