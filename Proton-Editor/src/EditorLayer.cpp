#include "EditorLayer.h"
#include "CameraController.h"
#include "Proton\Scene\SceneSerializer.h"
#include "Proton\Utils\PlatformUtils.h"
#include <Proton\Math\Math.h>
#include <Proton\Asset Loader\AssetManager.h>
#include "Proton\Scripting\ScriptEngine.h"

#include "ImGuizmo.h"
#include <DirectXMath.h>

#include <filesystem>

namespace Proton
{
	EditorLayer::EditorLayer()
		:
		Layer("EditorLayer")
	{
		projectPath = CoreUtils::CORE_PATH_STR + "Proton-Editor\\assets";
	}

	void EditorLayer::OnAttach()
	{
		m_IconPlay = Texture2D::Create("Resources/icons/PlayButton.png");
		m_IconStop = Texture2D::Create("Resources/icons/StopButton.png");
		m_IconPause = Texture2D::Create("Resources/icons/PauseButton.png");
		m_IconStep = Texture2D::Create("Resources/icons/StepButton.png");

		AssetManager::SetProjectPath(projectPath);
		AssetManager::ScanProject();

		Application::Get().GetWindow().ShowCursor();

		FramebufferDescription desc;
		desc.Width = 1280;
		desc.Height = 720;
		desc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH };
		desc.ClearColor = new float[4]{ 0.02f, 0.07f, 0.2f, 1 };
		m_SceneRenderer = CreateScope<SceneRenderer>(m_ActiveScene, desc);

		m_EditorCam = EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f);

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
		}
		else
		{
			//TODO: Prompt user to select a directory
			NewProject();
			//NewScene();
			//ModelCreator::CreateModelEntity(CoreUtils::CORE_PATH_STR + "Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj", *m_ActiveScene);

			//ModelCreator::CreateModelEntity(CoreUtils::CORE_PATH_STR + "Proton-Editor\\assets\\Models\\Sponza\\sponza.obj", *m_ActiveScene);
			//m_PointLight = m_ActiveScene->CreateEntity("Point Light");
			//m_PointLight.AddComponent<LightComponent>();
		}

		m_EditorScene = m_ActiveScene;

		SceneHierarchyPanel::SetContext(m_ActiveScene);
		AssetViewerPanel::SetProjectPath(projectPath);
		AssetViewerPanel::SetContext(m_ActiveScene);

		AssetViewerPanel::SetOpenSceneFunction(std::bind(static_cast<void(EditorLayer::*)(const std::filesystem::path&)>(&EditorLayer::OpenScene), this, std::placeholders::_1));

		//D:\\Dev\\Proton\\Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj
		//D:\\Dev\\Proton\\Proton-Editor\\assets\\cube.obj
		//ModelCreator::CreateModelEntity("D:\\Dev\\Proton\\Proton-Editor\\assets\\Models\\nano_textured\\nanosuit.obj", *m_ActiveScene);

		//
		//m_CameraEntity.AddComponent<CameraComponent>();
		//
		//m_CameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();*/
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

		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_ActiveScene->OnEditorUpdate(ts);
				m_SceneRenderer->Render(m_EditorCam.GetViewMatrix(), m_EditorCam.GetProjection());
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnRuntimeUpdate(ts);
				Entity cameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();
				if (cameraEntity != Entity::Null)
				{
					CameraComponent& camera = cameraEntity.GetComponent<CameraComponent>();
					TransformComponent& transform = cameraEntity.GetComponent<TransformComponent>();
					DirectX::XMMATRIX viewMatrix = XMMatrixInverse(nullptr, DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z) *
						XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&transform.position)));
					m_SceneRenderer->Render(viewMatrix, camera.Camera.GetProjection());
				}
				break;
			}
		}
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

	static DirectX::XMFLOAT3 threeaxisrot(float r11, float r12, float r21, float r31, float r32) {
		DirectX::XMFLOAT3 res;
		res.x = asin(r21);
		res.y = atan2(r11, r12);
		res.z = atan2(r31, r32);
		return res;
	}

	enum RotSeq { zyx, zyz, zxy, zxz, yxz, yxy, yzx, yzy, xyz, xyx, xzy, xzx };

	static DirectX::XMFLOAT3 QuatToEul(DirectX::XMVECTOR quaternion)
	{
		DirectX::XMFLOAT4 q;
		DirectX::XMStoreFloat4(&q, quaternion);

		float r11, r12, r21, r31, r32;
		r11 = 2 * (q.x * q.z + q.w * q.y);
		r12 = q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z;
		r21 = -2 * (q.y * q.z - q.w * q.x);
		r31 = 2 * (q.x * q.y + q.w * q.z);
		r32 = q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z;

		return { asin(r21), atan2(r11, r12), atan2(r31, r32) };
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
		ImGui::Begin("Scene", 0, ImGuiDockNodeFlags_AutoHideTabBar);
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

		if (viewportPanelSize.x != m_ViewportSize.x || viewportPanelSize.y != m_ViewportSize.y)
		{
			m_SceneRenderer->Resize((uint32_t)viewportPanelSize.x * m_AntiAliasing, (uint32_t)viewportPanelSize.y * m_AntiAliasing);
			m_ViewportSize = viewportPanelSize;

			m_ActiveScene->OnViewportResize(m_ViewportSize.x * m_AntiAliasing, m_ViewportSize.y * m_AntiAliasing);
			m_EditorCam.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
		}

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		m_UpdateEditorCam = m_ViewportFocused && m_SceneState == SceneState::Edit;

		ImGui::PopStyleVar();

		static bool texID = false;

		if (Input::IsKeyReleased(Key::Backspace) && ImGui::IsWindowFocused())
			texID = !texID;

		//Disable blending when drawing the image in order to avoid blending with the Window background if the texture has alpha
		ImDrawCallback disableBlend = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) { RenderCommand::DisableBlending(); };
		ImDrawCallback enableBlend = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) { RenderCommand::EnableBlending(); };
		
		ImGui::GetCurrentWindow()->DrawList->AddCallback(disableBlend, nullptr);
		ImGui::Image(m_SceneRenderer->GetRenderTextureID(texID), viewportPanelSize);
		ImGui::GetCurrentWindow()->DrawList->AddCallback(enableBlend, nullptr);

		// Guizmos
		Entity selectedEntity = SceneHierarchyPanel::Get().GetSelectedEntity();
		if (selectedEntity && m_GuizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			float windowWidth = ImGui::GetWindowWidth();
			float windowHeight = ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			DirectX::XMMATRIX cameraView = m_EditorCam.GetViewMatrix();
			DirectX::XMMATRIX cameraProjection = m_EditorCam.GetProjection();

			// Entity Transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			//TODO: Use Math SIMD functions to do
			DirectX::XMMATRIX transform = tc.GetTransformMatrix();

			ImGuizmo::Manipulate((float*)&cameraView, (float*)&cameraProjection, 
				(ImGuizmo::OPERATION)m_GuizmoType, ImGuizmo::LOCAL, (float*)&transform);

			if (ImGuizmo::IsUsing())
			{
				m_UpdateEditorCam = false;
				DirectX::XMVECTOR translation;
				DirectX::XMVECTOR rotation;
				DirectX::XMVECTOR scale;
				DirectX::XMMatrixDecompose(&scale, &rotation, &translation, transform);

				DirectX::XMStoreFloat3(&tc.position, translation);

				DirectX::XMFLOAT4 q;
				DirectX::XMStoreFloat4(&q, rotation);

				tc.rotation = QuatToEul(rotation);

				DirectX::XMStoreFloat3(&tc.scale, scale);//*/
			}
		}

		ImGui::End();

		SceneHierarchyPanel::Get().OnImGuiRender();
		AssetViewerPanel::OnImGuiRender();
		ConsolePanel::OnImGuiRender();

		UI_Toolbar();
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

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload Assembly", "Ctrl+R"))
				{
					ScriptEngine::ReloadAssembly();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	void EditorLayer::UI_Toolbar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 2));

		auto& colors = ImGui::GetStyle().Colors;
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
		auto& buttonActive = colors[ImGuiCol_ButtonActive];
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		float size = ImGui::GetWindowHeight() - 4.0f;

		{
			Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
			ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
			if (ImGui::ImageButton(icon->GetTexturePointer(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
			{
				if (m_SceneState == SceneState::Edit)
					OnScenePlay();
				else if (m_SceneState == SceneState::Play)
					OnSceneStop();
			}
		}

		if (m_SceneState != SceneState::Edit)
		{
			ImGui::SameLine();
			bool isPaused = m_ActiveScene->IsPaused();
			const ImVec4 unpausedTint(1, 1, 1, 1);
			const ImVec4 pausedTint(0.7f, 0.7f, 0.7f, 1.0f);

			if (ImGui::ImageButton(m_IconPause->GetTexturePointer(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 0), isPaused ? pausedTint : unpausedTint))
			{
				m_ActiveScene->SetPaused(!isPaused);
			}

			if (isPaused)
			{
				ImGui::SameLine();
				if (ImGui::ImageButton(m_IconStep->GetTexturePointer(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
				{
					m_ActiveScene->Step();
				}
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

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
		case Key::R:
			if (control)
				ScriptEngine::ReloadAssembly();
			break;
		}

		return true;
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Proton Project (*.pproj)\0*.pproj\0");

		if (!filepath.empty())
		{
			OpenProject(filepath);
		}
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
			OpenScene(startScenePath);
		}
	}

	void EditorLayer::SaveProject()
	{
		//Project::SaveActive();
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x * m_AntiAliasing, (uint32_t)m_ViewportSize.y * m_AntiAliasing);
		m_EditorCam.SetViewportSize(m_ViewportSize.x * m_AntiAliasing, m_ViewportSize.y * m_AntiAliasing);
		m_SceneRenderer->SetScene(m_ActiveScene);

		SceneHierarchyPanel::SetContext(m_ActiveScene);
		AssetViewerPanel::SetProjectPath(projectPath);
		AssetViewerPanel::SetContext(m_ActiveScene);

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		m_CameraEntity.AddComponent<CameraComponent>();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Proton Scene (*.proton)\0*.proton\0");

		if (!filepath.empty())
		{
			OpenScene(filepath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (path.extension() != ".proton")
		{
			PT_CORE_ERROR("Could not open scene {0} - Not a Proton scene file", path.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);

		if (serializer.Deserialize(path.string(), m_EditorCam))
		{
			saveFilePath = path.string();
			m_EditorScene = newScene;
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x * m_AntiAliasing, (uint32_t)m_ViewportSize.y * m_AntiAliasing);
			m_EditorCam.SetViewportSize(m_ViewportSize.x * m_AntiAliasing, m_ViewportSize.y * m_AntiAliasing);

			m_ActiveScene = m_EditorScene;

			SceneHierarchyPanel::SetContext(m_ActiveScene);
			AssetViewerPanel::SetProjectPath(projectPath);
			AssetViewerPanel::SetContext(m_ActiveScene);

			m_CameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();

			m_SceneRenderer->SetScene(m_ActiveScene);
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!saveFilePath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(saveFilePath, m_EditorCam);
		}
		else
		{
			SaveSceneAs();
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Proton Scene (*.proton)\0*.proton\0");

		if (!filepath.empty())
		{
			std::filesystem::path path = filepath;
			if (path.extension() != ".proton")
				path += ".proton";

			filepath = path.string();

			saveFilePath = filepath;
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath, m_EditorCam);
		}
	}

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		m_UpdateEditorCam = false;

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		SceneHierarchyPanel::SetContext(m_ActiveScene);
		m_SceneRenderer->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_UpdateEditorCam = true;

		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene = m_EditorScene;

		SceneHierarchyPanel::SetContext(m_ActiveScene);
		m_SceneRenderer->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit)
			return;

		m_ActiveScene->SetPaused(true);
	}
}