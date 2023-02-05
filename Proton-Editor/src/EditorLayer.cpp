#include "EditorLayer.h"
#include "CameraController.h"
#include "Proton\Scene\SceneSerializer.h"
#include "Proton\Utils\PlatformUtils.h"
#include <Proton\Math\Math.h>
#include <Proton\Asset Loader\AssetManager.h>

//TEMP
#include "CompileTimeTests.h"
#include <DirectXMath.h>

#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace Proton
{
	EditorLayer::EditorLayer()
		:
		Layer("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		std::string projectPath = __FILE__;
		int directoryCounter = 0;
		while (directoryCounter < 3)
		{
			if (projectPath.back() == '\\')
				directoryCounter++;

			projectPath.pop_back();
		}

		AssetManager::SetProjectPath(projectPath);
		AssetManager::ScanProject();

		Application::Get().GetWindow().ShowCursor();

		FramebufferDescription desc;
		desc.Width = 1280;
		desc.Height = 720;
		desc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH };
		desc.ClearColor = new float[4]{ 0.02f, 0.07f, 0.2f, 1 };

		m_ActiveScene = CreateRef<Scene>();
		m_EditorCam = EditorCamera(45.0f, 1.778f, 0.1f, 1000.0f);
		m_SceneRenderer = CreateScope<SceneRenderer>(m_ActiveScene, desc);

		SceneHierarchyPanel::SetScene(m_ActiveScene);
		AssetViewerPanel::SetProjectPath(projectPath);
		AssetViewerPanel::SetScene(m_ActiveScene);


		//D:\\Dev\\Proton\\Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj
		//D:\\Dev\\Proton\\Proton-Editor\\assets\\cube.obj
		ModelCreator::CreateModelEntity("D:\\Dev\\Proton\\Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj", *m_ActiveScene);
		ModelCreator::CreateModelEntity("D:\\Dev\\Proton\\Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj", *m_ActiveScene);

		//m_Nanosuit = ModelCreator::CreateModelEntity("C:\\Dev\\Proton\\Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj", m_ActiveScene.get());
		//
		//m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		//m_PointLight = m_ActiveScene->CreateEntity("Point Light");
		//
		//m_CameraEntity.AddComponent<CameraComponent>();
		//m_PointLight.AddComponent<LightComponent>();
		//
		//m_CameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();

		//Shader Parsing (To Be continued later)
		/*std::string shaderPath = "C:\\Dev\\Proton\\Proton-Editor\\assets\\Shaders\\VertexShader.shader";

		std::vector<Token> tokens = ShaderParser::LexShader(shaderPath);

		std::string outputString = "";
		uint32_t currentLine = 1;
		for (auto token : tokens)
		{
			if (currentLine == token.GetLine())
			{
				outputString += token.ToString() + "|";
			}
			else if (token.GetLine() > currentLine)
			{
				while (currentLine < token.GetLine())
				{
					outputString += "\n";
					currentLine++;
				}
				outputString += token.ToString() + "|";
			}
		}

		std::ofstream stream("C:\\Dev\\Proton\\Proton-Editor\\assets\\Shaders\\shader.txt");
		stream << outputString;
		stream.close();

		ShaderProfile* profile = ShaderParser::ParseShader(shaderPath);
		if (profile)
		{
			DrawScope(profile->scope, 0);
		}
		else
		{
			for (auto error : ShaderParser::GetErrors())
			{
				LOG_ERROR(error.first->ToString() + " at line " + std::to_string(error.second));
			}
		}*/
	}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::OnUpdate(TimeStep ts)
	{
		PT_PROFILE_FUNCTION();

		if (Input::IsKeyReleased(Key::Escape))
		{
			cursor = !cursor;
		}

		if (Input::IsKeyPressed(Key::Escape))
		{
			if (cursor)
			{
				Application::Get().GetWindow().HideCursor();
				Application::Get().GetWindow().DisableImGuiCursor();
				Application::Get().GetWindow().ConfineCursor();
			}
			else
			{
				Application::Get().GetWindow().ShowCursor();
				Application::Get().GetWindow().EnableImGuiCursor();
				Application::Get().GetWindow().FreeCursor();
			}
		}

		static bool vSync = true;

		if (Input::IsKeyReleased(Key::D0))
		{
			vSync = !vSync;
			Application::Get().GetWindow().SetVSync(vSync);
		}

		//TEMP
		/*m_ActiveScene->DrawDebugLine({ 0, 10, 0 }, { 0, 20, 0 }, 0, 1, 0);
		m_ActiveScene->DrawDebugLine({ 0, 20, 0 }, { 10, 20, 0 }, 0, 1, 0);
		m_ActiveScene->DrawDebugLine({ 10, 20, 0 }, { 10, 10, 0 }, 0, 1, 0);
		m_ActiveScene->DrawDebugLine({ 10, 10, 0 }, { 0, 10, 0 }, 0, 1, 0);*/

		if (m_UpdateEditorCam)
			m_EditorCam.OnUpdate(ts);

		m_ActiveScene->OnEditorUpdate(ts, m_EditorCam);

		m_SceneRenderer->Render(m_EditorCam);
	}

	//TEMP
	static void DrawFloat3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2, 0, false);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3, lineHeight };

		//X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Z
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void EditorLayer::OnImGuiRender()
	{
		//ImGui Dockspace
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->GetWorkPos());
			ImGui::SetNextWindowSize(viewport->GetWorkSize());
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSize = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;

		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("DockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSize;

		//Custom Windows
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Scene");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

		if (viewportPanelSize.x != m_ViewportSize.x || viewportPanelSize.y != m_ViewportSize.y)
		{
			m_SceneRenderer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
			m_ViewportSize = viewportPanelSize;

			m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
			m_EditorCam.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}

		m_UpdateEditorCam = ImGui::IsWindowFocused();

		ImGui::PopStyleVar();

		static bool texID = false;

		if (Input::IsKeyReleased(Key::Backspace) && ImGui::IsWindowFocused())
			texID = !texID;

		ImGui::Image(m_SceneRenderer->GetRenderTextureID(texID), viewportPanelSize);
		ImGui::End();

		SceneHierarchyPanel::OnImGuiRender();
		AssetViewerPanel::OnImGuiRender();
		ConsolePanel::OnImGuiRender();
		//

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}

				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					SaveScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				if (ImGui::MenuItem("Exit")) Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if(m_UpdateEditorCam)
			m_EditorCam.OnEvent(e);

		if (e.IsEventType(EventType::WindowClose))
		{
			
		}

		if (e.IsEventType(EventType::FileDragDrop))
		{
			AssetViewerPanel::AddFile(((FileDragDropEvent&)e).GetFilePath());
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(PT_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	void EditorLayer::DrawScope(const ScopeNode* scope, uint32_t scopeLevel)
	{
		std::string tabs = "";
		for (int i = 0; i < scopeLevel; i++) tabs += "=>";

		for (int i = 0; i < scope->opNodes.size(); i++)
		{
			const auto node = scope->opNodes[i];

			std::string showNode = tabs + node->ToString();

			LOG_TRACE(showNode);

			if (node->type == OpNode::OpType::Scope)
				DrawScope(static_cast<ScopeNode*>(node), scopeLevel + 1);

			if (Scoped* scoped = dynamic_cast<Scoped*>(node))
				DrawScope(scoped->scope, scopeLevel + 1);
		}
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 1)
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl) || Input::IsKeyPressed(Key::Control);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift) || Input::IsKeyPressed(Key::Shift);

		switch (e.GetKeyCode())
		{
		case Key::N:
			if (control)
				NewScene();
			break;
		case Key::O:
			if (control)
				OpenScene();
			break;
		case Key::S:
			if (control && shift)
				SaveSceneAs();
			else if (control)
				SaveScene();
			break;
		}

		return true;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_EditorCam.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		m_SceneRenderer->SetScene(m_ActiveScene);

		SceneHierarchyPanel::SetScene(m_ActiveScene);
		AssetViewerPanel::SetProjectPath("D:\\Dev\\Proton\\Proton-Editor");
		AssetViewerPanel::SetScene(m_ActiveScene);

		m_CameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Proton Scene (*.proton)\0*.proton\0");

		if (!filepath.empty())
		{
			saveFilePath = filepath;
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCam.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_SceneRenderer->SetScene(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(filepath, m_EditorCam);

			SceneHierarchyPanel::SetScene(m_ActiveScene);
			AssetViewerPanel::SetProjectPath("D:\\Dev\\Proton\\Proton-Editor");
			AssetViewerPanel::SetScene(m_ActiveScene);

			m_CameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!saveFilePath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(saveFilePath, m_EditorCam);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Proton Scene (*.proton)\0*.proton\0");

		if (!filepath.empty())
		{
			saveFilePath = filepath;
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath, m_EditorCam);
		}
	}
}