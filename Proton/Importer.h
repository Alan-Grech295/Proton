#pragma once
#include "Proton/Model/Model.h"
#include <filesystem>

namespace Proton
{
	class ModelImporter
	{
	public:
		struct ImporterSettings
		{
		};

		static Ref<Model> ImportModel(const std::filesystem::path& path, const ImporterSettings& settings);
	};
}

