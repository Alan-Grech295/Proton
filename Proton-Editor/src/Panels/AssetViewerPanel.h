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
		AssetViewerPanel(const std::string& assetPath);
		~AssetViewerPanel();

		void OnImGuiRender();

		std::filesystem::path& GetSelectedPath() { return m_SelectedPath; }
		void SetScene(const Ref<Scene> scene) { m_ActiveScene = scene; }

		void AddFile(std::filesystem::path filePath);

		static AssetViewerPanel& Get() { return *assetViewer; }
	private:
		void DrawDirectories();
		void DrawDirectory(const std::filesystem::path& pathToScan);
		void DrawFiles();
		void SetSelectedPath(const std::filesystem::path& path);
	private:
		std::string startPath;
		std::filesystem::path m_SelectedPath;
		std::string m_SelectedPathString;
		Scope<Texture2D> folderIcon;
		Scope<Texture2D> fileIcon;
		Ref<Scene> m_ActiveScene;

		std::string m_SelectedItem;

		static AssetViewerPanel* assetViewer;
	};
}