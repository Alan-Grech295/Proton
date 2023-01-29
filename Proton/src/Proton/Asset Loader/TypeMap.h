#pragma once

namespace Proton
{
	typedef unsigned char byte;

#define ELEMENT_TYPES \
				X(Bool)\
				X(Byte)\
				X(Int16)\
				X(Int32)\
				X(Int64)\
				X(UInt16)\
				X(UInt32)\
				X(UInt64)\
				X(Float)\
				X(Double)\
				X(String)

	enum class Type {
		None = 0,
#define X(el) el,
		ELEMENT_TYPES
#undef X
		Struct,
		Array,
		Pointer
	};

	template<Type type>
	struct Map
	{
		static constexpr bool valid = false;
	};
	template<> struct Map<Type::Bool>
	{
		using SysType = bool; // type used in the CPU side
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

	template<> struct Map<Type::UInt16>
	{
		using SysType = uint16_t; // type used in the CPU side
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
	template<> struct Map<Type::UInt32>
	{
		using SysType = uint32_t; // type used in the CPU side
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
	template<> struct Map<Type::UInt64>
	{
		using SysType = uint64_t; // type used in the CPU side
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

	static uint32_t GetSize(Type type)
	{
		assert("Cannot get size of string" && type != Type::String);

		switch (type)
		{
#define X(el) case Type::el: return Map<Type::el>::SizeBytes(nullptr);
			ELEMENT_TYPES
#undef X
		}

		assert("Invalid type" && false);
		return 0;
	}
}