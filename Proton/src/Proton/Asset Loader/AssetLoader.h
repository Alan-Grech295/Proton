#pragma once
#include <filesystem>
#include <fstream>
#include <DirectXTex.h>

namespace Proton
{
	enum class AssetType { Model = 0, Image = 1};
	class AssetLoader
	{
	public:
		static char* ReadData(const std::filesystem::path& modelPath);
	};
}