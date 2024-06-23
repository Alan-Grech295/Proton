#pragma once
#include <Proton.h>
#include <string>
#include <Proton\Core\Core.h>
#include <filesystem>

namespace Proton
{
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();
		~ContentBrowserPanel();

		void OnImGuiRender();

		std::filesystem::path& GetSelectedPath() { return m_SelectedPath; }
		void SetContext(const Ref<Scene> scene) { m_ActiveScene = scene; m_SelectedPath = Project::GetAssetDirectory(); }

		void SetOpenSceneFunction(std::function<bool(const std::filesystem::path&)> func) { m_OpenSceneFunc = func; }

		void AddFile(std::filesystem::path filePath);
	private:
		void DrawDirectories();
		void DrawDirectory(const std::filesystem::path& pathToScan);
		void DrawFiles();
		void SetSelectedPath(const std::filesystem::path& path);
	private:
		std::filesystem::path m_SelectedPath;
		Ref<Texture2D> folderIcon;
		Ref<Texture2D> fileIcon;
		Ref<Scene> m_ActiveScene;

		std::string m_SelectedItem;

		std::function<bool(const std::filesystem::path&)> m_OpenSceneFunc;
	};
}