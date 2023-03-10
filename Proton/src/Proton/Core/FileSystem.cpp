#include "ptpch.h"
#include "FileSystem.h"

namespace Proton
{
	Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
	{
		std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

		if (!stream)
		{
			PT_CORE_ERROR("Error opening file");
			return {};
		}

		std::streampos end = stream.tellg();
		stream.seekg(0, std::ios::beg);
		uint32_t size = end - stream.tellg();

		if (size == 0)
		{
			PT_CORE_ERROR("File is empty");
			return {};
		}

		Buffer buffer(size);
		stream.read(buffer.As<char>(), size);
		stream.close();

		return buffer;
	}
}