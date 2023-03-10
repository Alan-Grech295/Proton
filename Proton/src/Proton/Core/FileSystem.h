#pragma once
#include "Buffer.h"
#include <filesystem>

namespace Proton
{
	//TODO: Move to platforms
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
		static std::string ReadFileText(const std::filesystem::path& filepath);
	};
}