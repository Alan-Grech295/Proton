#include "AssetViewerPanel.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace fs = std::filesystem;

namespace Proton
{
	AssetViewerPanel* AssetViewerPanel::assetViewer = nullptr;
	AssetViewerPanel::AssetViewerPanel()
	{

	}

	AssetViewerPanel::AssetViewerPanel(const std::string& assetPath)
		:
		startPath(assetPath),
		m_SelectedPath(assetPath),
		folderIcon(Texture2D::CreateUnique("icons\\Folder-icon.png")),
		fileIcon(Texture2D::CreateUnique("icons\\Document-Blank-icon.png"))
	{
		assetViewer = this;
	}

	AssetViewerPanel::~AssetViewerPanel()
	{

	}

	void AssetViewerPanel::OnImGuiRender()
	{
		ImGui::Begin("Asset Viewer");
		ImGui::Columns(2);

		DrawDirectories();

		ImGui::NextColumn();

		DrawFiles();

		ImGui::Columns(1);
		ImGui::End();
	}

	void AssetViewerPanel::AddFile(std::filesystem::path filePath)
	{
		std::ifstream inStream(filePath, std::ios::in | std::ios::binary);

		std::string savePath = m_SelectedPath.string() + "\\";
		std::string saveName = filePath.filename().replace_extension().string();
		std::string extension = filePath.extension().string();

		if (fs::exists(savePath + saveName + extension))
		{
			uint32_t count = 1;
			while (fs::exists(savePath + saveName + std::to_string(count) + extension))
			{
				count++;
			}

			savePath = savePath + saveName + std::to_string(count) + extension;
		}
		else
		{
			savePath = savePath + saveName + extension;
		}

		std::ofstream outStream(savePath, std::ios::out | std::ios::binary);

		if (inStream && outStream)
		{
			std::filebuf* fileBuf = inStream.rdbuf();
			uint64_t size = fileBuf->pubseekoff(0, inStream.end, inStream.in);
			fileBuf->pubseekpos(0, inStream.in);

			char* buffer = new char[size];

			fileBuf->sgetn(buffer, size);

			outStream.write(buffer, size);
		}

		PT_CORE_TRACE("Added file '{0}' to '{1}'", filePath.string(), savePath);

		inStream.close();
		outStream.close();
	}

	void AssetViewerPanel::DrawDirectories()
	{
		for (const auto& entry : fs::directory_iterator(startPath)) {
			if (entry.is_directory()) {
				DrawDirectory(entry);
			}
		}
	}

	void AssetViewerPanel::DrawDirectory(const fs::path& pathToScan)
	{
		std::string pathString = pathToScan.string();
		bool isLeaf = true;
		bool defaultOpen = pathString.size() != m_SelectedPathString.size() && pathString == m_SelectedPathString.substr(0, pathString.size());

		for (const auto& entry : fs::directory_iterator(pathToScan)) {
			const auto filenameStr = entry.path().filename().string();
			if (entry.is_directory()) {

				isLeaf = false;
				break;
			}
		}

		ImGuiTreeNodeFlags flags =  ImGuiTreeNodeFlags_OpenOnArrow |
									ImGuiTreeNodeFlags_OpenOnDoubleClick |
									(isLeaf ? ImGuiTreeNodeFlags_Leaf : 0) |
									ImGuiTreeNodeFlags_SpanAvailWidth |
									(defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0);

		bool opened = ImGui::TreeNodeEx(pathToScan.string().c_str(), flags, pathToScan.filename().string().c_str());

		if (ImGui::IsItemClicked())
		{
			SetSelectedPath(pathToScan);
		}

		if (opened)
		{
			for (const auto& entry : fs::directory_iterator(pathToScan)) {
				const auto filenameStr = entry.path().filename().string();
				if (entry.is_directory()) {

					DrawDirectory(entry);
				}
			}

			ImGui::TreePop();
		}	
	}

	static void NamedButton(void* textureID, const std::string& label, std::string& selectedItem, bool& doubleClicked, bool& selected,  const ImVec2& size, const ImVec2& padding = { 8, 8 })
	{
		ImGui::PushID(label.c_str());

		ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
		ImVec2 fullSize = ImVec2{ size.x + padding.x, size.y + ImGui::CalcTextSize(label.c_str(), (const char*)0, false, size.x).y + padding.y };

		ImVec2 cursorPos = ImGui::GetCursorPos();

		ImGui::BeginGroup();

		selected = ImGui::Selectable("", selectedItem == label, ImGuiSelectableFlags_AllowItemOverlap, fullSize);

		doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		if (selected)
			selectedItem = label;

		ImGui::SetCursorPos(ImVec2{ cursorPos.x + padding.x / 2.0f, cursorPos.y });

		ImGui::Image(textureID, size);

		ImGui::Text("");
		ImGui::SameLine(max(0.0f, ((size.x - textSize.x) * 0.5f)) + padding.x / 2.0f);

		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + size.x);
		ImGui::TextWrapped(label.c_str());
		ImGui::PopTextWrapPos();

		ImGui::EndGroup();

		ImGui::PopID();
	}

	void AssetViewerPanel::DrawFiles()
	{
		int maxPerLine = ImGui::GetContentRegionAvail().x / 64.0f;
		int onCurLine = 0;
		ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_WindowBg]);
		for (const auto& entry : fs::directory_iterator(m_SelectedPath)) {
			const auto filenameStr = entry.path().filename().string();

			if (entry.is_directory()) 
			{
				bool selected = false;
				bool doubleClicked = false;

				NamedButton(folderIcon->GetTexturePointer(), filenameStr, m_SelectedItem, doubleClicked, selected, ImVec2{ 64, 64 });

				if (doubleClicked)
				{
					PT_CORE_TRACE(filenameStr);
					SetSelectedPath(entry);
				}
			}
			else 
			{
				if (entry.path().extension() != ".asset" && entry.path().extension() != ".prefab")
				{
					bool selected = false;
					bool doubleClicked = false;
					NamedButton(fileIcon->GetTexturePointer(), filenameStr, m_SelectedItem, doubleClicked, selected, ImVec2{ 64, 64 });
					if (doubleClicked)
					{
						std::system(entry.path().string().c_str());
					}
				}
				else if (entry.path().extension() == ".prefab")
				{
					bool selected = false;
					bool doubleClicked = false;

					NamedButton(fileIcon->GetTexturePointer(), filenameStr, m_SelectedItem, doubleClicked, selected, ImVec2{ 64, 64 });

					if (doubleClicked)
					{
						ModelCreator::CreatePrefabEntity(entry.path().string(), m_ActiveScene.get());
					}
				}
			}

			onCurLine++;
			if (onCurLine >= maxPerLine)
			{
				onCurLine = 0;
			}
			else
			{
				ImGui::SameLine(0, 5);
			}
			//ImGui::Text(filenameStr.c_str());
		}

		ImGui::PopStyleColor();
	}

	//TODO: Improve Splitting
	void AssetViewerPanel::SetSelectedPath(const fs::path& path)
	{
		m_SelectedPath = path;

		m_SelectedPathString = path.string();
		PT_CORE_TRACE("Selected path {0}", m_SelectedPathString);
	}
}
