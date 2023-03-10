#pragma once

#include <stdint.h>
#include <memory.h>

namespace Proton
{
	//Non-owning raw buffer
	struct Buffer
	{
		Buffer() = default;
		Buffer(uint64_t size)
		{
			Allocate(size);
		}

		Buffer(const Buffer&) = default;

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0;
		}

		void Allocate(uint64_t size)
		{
			Release();

			Data = new uint8_t[size];
			Size = size;
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		operator bool() const 
		{
			return (bool)Data;
		}

	public:
		uint8_t* Data = nullptr;
		uint64_t Size = 0;
	};

	struct ScopedBuffer : public Buffer
	{
		ScopedBuffer(Buffer buffer)
			: 
			Buffer(buffer)
		{
		}

		~ScopedBuffer()
		{
			Release();
		}
	};
}