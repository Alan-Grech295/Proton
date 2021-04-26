#include "ptpch.h"
#include "AssetLoader.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Platform\DirectX 11\DirectXTexture.h"

namespace fs = std::filesystem;

namespace Proton
{
	char* AssetLoader::ReadAsset(const std::filesystem::path& assetPath)
	{
		return nullptr;
	}

	char* AssetLoader::ReadData(const std::filesystem::path& modelPath)
	{
		std::ifstream inStream(modelPath, std::ios::out | std::ios::binary);

		if (!inStream)
		{
			PT_ERROR("Cannot open file {0}!", modelPath.string());
		}

		std::filebuf* fileBuf = inStream.rdbuf();
		std::size_t size = fileBuf->pubseekoff(0, inStream.end, inStream.in);
		fileBuf->pubseekpos(0, inStream.in);

		char* buffer = new char[size];

		fileBuf->sgetn(buffer, size);

		inStream.close();

		return buffer;
	}
}
