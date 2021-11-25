#pragma once
#include <Proton.h>
#include <string>
#include <Proton\Core\Core.h>
#include <filesystem>

namespace Proton
{
	class AssetViewerPanel
	{
	public:
		AssetViewerPanel();
		~AssetViewerPanel();

		static void OnImGuiRender();

		static std::filesystem::path& GetSelectedPath() { return Get().m_SelectedPath; }
		static void SetScene(const Ref<Scene> scene) { Get().m_ActiveScene = scene; }
		static void SetProjectPath(const std::string& path) { Get().startPath = path; Get().m_SelectedPath = path; }

		static void AddFile(std::filesystem::path filePath);
	private:
		void DrawDirectories();
		void DrawDirectory(const std::filesystem::path& pathToScan);
		void DrawFiles();
		void SetSelectedPath(const std::filesystem::path& path);

		static AssetViewerPanel& Get()
		{
			static AssetViewerPanel assetViewer;
			return assetViewer;
		}
	private:
		std::string startPath;
		std::filesystem::path m_SelectedPath;
		std::string m_SelectedPathString;
		Scope<Texture2D> folderIcon;
		Scope<Texture2D> fileIcon;
		Ref<Scene> m_ActiveScene;

		std::string m_SelectedItem;
	};
}