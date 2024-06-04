#pragma once
#include "Proton/Core/Core.h"

#include <string>
#include <filesystem>

namespace Proton
{
	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory;
		std::filesystem::path ScriptPath;
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetProjectDirectory()
		{
			PT_CORE_ASSERT(s_ActiveProject);
			return s_ActiveProject->m_ProjectDirectory;
		}

		static std::filesystem::path GetAssetDirectory()
		{
			PT_CORE_ASSERT(s_ActiveProject);
			return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
		}

		//TODO: Move to asset manager
		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			PT_CORE_ASSERT(s_ActiveProject);
			return GetAssetDirectory() / path;
		}

		// Assuming that a non-relative path is from the Asset directory
		static std::filesystem::path GetAssetRelativePath(const std::filesystem::path& path)
		{
			PT_CORE_ASSERT(s_ActiveProject);
			if(path.is_absolute())
				return std::filesystem::relative(path, GetAssetDirectory());

			return path;
		}

		ProjectConfig& GetConfig() 
		{ 
			return m_Config; 
		}

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
}