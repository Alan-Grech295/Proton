#include "EditorLayer.h"
#include "CameraController.h"
#include "Proton\Scene\SceneSerializer.h"
#include "Proton\Utils\PlatformUtils.h"
#include <Proton\Math\Math.h>
#include "Proton\Scripting\ScriptEngine.h"
#include "Proton/Scene/Entity.h"
#include "FileLoader.h"

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

		Application::Get().GetWindow().ShowCursor();

		FramebufferDescription desc;
		desc.Width = 1280;
		desc.Height = 720;
		desc.Attachments = { 
			{FramebufferTextureFormat::RGBA8, DirectX::XMFLOAT4{ 0.02f, 0.07f, 0.2f, 1 }},
			{FramebufferTextureFormat::RINT, -1},
			{FramebufferTextureFormat::RINT, -1},
			{FramebufferTextureFormat::DEPTH, 1.0f}
		};

		desc.ClearColor = new float[4]{ 0.02f, 0.07f, 0.2f, 1 };
		m_SceneRenderer = CreateScope<EditorSceneRenderer>(m_ActiveScene, desc);
		m_EditorCam = EditorCamera(45.0f, 1.778f, 0.1f, 10000.0f);

		m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
		m_ConsolePanel = CreateScope<ConsolePanel>();
		m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>();

		m_ContentBrowserPanel->SetOpenSceneFunction(std::bind(static_cast<bool(EditorLayer::*)(const std::filesystem::path&)>(&EditorLayer::OpenScene), this, std::placeholders::_1));

		auto commandLineArgs = Application::Get().GetSpecification().CommandLineArgs;
		if (commandLineArgs.Count > 1)
		{
			auto projectFilePath = commandLineArgs[1];
			OpenProject(projectFilePath);
			//NewProject();


			/*Ref<Model> model = assetManager.LoadAsset<Model>("Models\\Sponza\\sponza.obj");
			Model::CreateEntity(model, *m_ActiveScene);*/

			/*Ref<Model> nano = assetManager.LoadAsset<Model>("Models\\nano_textured\\nanosuit.obj");
			Model::CreateEntity(nano, *m_ActiveScene);*/

			/*Ref<Model> cube = assetManager.LoadAsset<Model>("Models\\cube.obj");
			Model::CreateEntity(cube, *m_ActiveScene);*/
		}
		else
		{
			//TODO: Prompt user to select a directory
			NewProject();
		}

		m_EditorScene = m_ActiveScene;
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

		if (Input::IsKeyReleased(Key::NUM_0))
		{
			vSync = !vSync;
			Application::Get().GetWindow().SetVSync(vSync);
		}

		if(m_UpdateEditorCamera)
			m_EditorCam.OnUpdate(ts);

		if (Input::IsMouseButtonPressed(0))
		{
			if (initClick)
			{
				initClick = false;
				dragging = false;
				mouseClickX = Input::GetMouseX();
				mouseClickY = Input::GetMouseY();
			}
			
			float dX = (mouseClickX - Input::GetMouseX());
			float dY = (mouseClickY - Input::GetMouseY());

			float sqrDist = dX * dX + dY * dY;

			if (sqrDist >= dragThreshold * dragThreshold)
			{
				dragging = true;
			}
		}
		else
		{
			initClick = true;
		}

		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_ActiveScene->OnEditorUpdate(ts);
				m_SceneRenderer->SetSelectedEntity(m_SceneHierarchyPanel->GetSelectedEntity());
				m_SceneRenderer->Render(m_EditorCam.GetViewMatrix(), m_EditorCam.GetProjection());
				m_SceneRenderer->RenderPickOutline();
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
		ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
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

		auto viewportOffset = ImGui::GetCursorPos();

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);
		m_UpdateEditorCamera = m_ViewportHovered && m_SceneState != SceneState::Play;

		ImGui::PopStyleVar();

		static bool texID = false;

		if (Input::IsKeyReleased(Key::Backspace) && ImGui::IsWindowFocused())
			texID = !texID;

		//Disable blending when drawing the image in order to avoid blending with the Window background if the texture has alpha
		ImDrawCallback disableBlend = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) { RenderCommand::DisableBlending(); };
		ImDrawCallback enableBlend = [](const ImDrawList* parent_list, const ImDrawCmd* cmd) { RenderCommand::EnableBlending(); };
		
		ImGui::GetCurrentWindow()->DrawList->AddCallback(disableBlend, nullptr);
		// Draw viewport
		ImGui::Image(m_SceneRenderer->GetRenderTextureID(texID ? 2 : 0), viewportPanelSize);

		if (ImGui::BeginDragDropTarget())
		{
			ImGuiDragDropFlags targetFlags = 0;
			targetFlags |= ImGuiDragDropFlags_AcceptNoDrawDefaultRect; // Don't display the yellow rectangle

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE", targetFlags))
			{
				auto [mx, my] = ImGui::GetMousePos();
				mx -= m_ViewportBounds[0].x;
				my -= m_ViewportBounds[0].y;

				int mouseX = (int)mx;
				int mouseY = (int)my;

				float depthPixelData = m_SceneRenderer->GetFrameBuffer()->ReadDepthPixel<float>(mouseX, mouseY);
				float camNear = m_EditorCam.GetNearClip();
				float camFar = m_EditorCam.GetFarClip();
				float actualDist = camFar * camNear / (camFar - depthPixelData * (camFar - camNear));

				DirectX:XMVECTOR uv = DirectX::XMVectorSet(mx / viewportPanelSize.x * 2.0f - 1.0f, (viewportPanelSize.y - my) / viewportPanelSize.y * 2.0f - 1.0f, 0, 1);

				DirectX::XMVECTOR direction = DirectX::XMVector4Transform(uv,
					DirectX::XMMatrixInverse(nullptr, m_EditorCam.GetProjection()));

				DirectX::XMFLOAT4 dir4;
				DirectX::XMStoreFloat4(&dir4, direction);

				direction = DirectX::XMVectorSet(dir4.x, dir4.y, dir4.z, 0);
				direction = DirectX::XMVector4Transform(direction, DirectX::XMMatrixInverse(nullptr, m_EditorCam.GetViewMatrix()));
				direction = DirectX::XMVector3Normalize(direction);
				
				DirectX::XMFLOAT3 placementPos;

				DirectX::XMStoreFloat3(&placementPos, 
					DirectX::XMVectorAdd(m_EditorCam.GetPosition(), 
										 DirectX::XMVectorScale(direction, actualDist)));

				std::string path = std::string((char*)payload->Data, payload->DataSize);
				FileLoader::LoadFile(path, placementPos, *m_ActiveScene);
			}
			ImGui::EndDragDropTarget();
		}

		ImGui::GetCurrentWindow()->DrawList->AddCallback(enableBlend, nullptr);

		auto windowSize = ImGui::GetWindowSize();
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
		m_ViewportBounds[0] = minBound;
		m_ViewportBounds[1] = maxBound;

		// Guizmos
		Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
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
			DirectX::XMMATRIX transform = selectedEntity.LocalToWorld();

			//Snapping
			bool snap = Input::IsKeyPressed(Key::Control);
			float snapValue = 0.5f;
			if (m_GuizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate((float*)&cameraView, (float*)&cameraProjection, 
				(ImGuizmo::OPERATION)m_GuizmoType, ImGuizmo::LOCAL, (float*)&transform, 
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				m_UpdateEditorCamera = false;
				transform = selectedEntity.WorldToLocal(transform);
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

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		ImVec2 viewportSize = { m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y };
		//Comment to unflip
		//my = m_ViewportSize.y - my;

		int mouseX = (int)mx;
		int mouseY = (int)my;

		// Entity Picking
		if (!ImGuizmo::IsOver() && Input::IsMouseButtonReleased(0) && !dragging &&
			mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)m_ViewportSize.y)
		{
			int entityID = m_SceneRenderer->GetFrameBuffer()->ReadPixel<int>(1, mouseX, mouseY);
			if (entityID != -1)
			{
				m_SceneHierarchyPanel->SelectEntity(Entity((entt::entity)entityID, m_ActiveScene.get()));

			}
		}

		ImGui::End();

		m_SceneHierarchyPanel->OnImGuiRender();
		m_ContentBrowserPanel->OnImGuiRender();
		m_ConsolePanel->OnImGuiRender();

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
		if (m_UpdateEditorCamera)
			m_EditorCam.OnEvent(e);

		if (e.IsEventType(EventType::WindowClose))
		{

		}

		if (e.IsEventType(EventType::FileDragDrop))
		{
			m_ContentBrowserPanel->AddFile(((FileDragDropEvent&)e).GetFilePath());
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(PT_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
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
		{
			if (control)
				NewScene();
			break;
		}
		case Key::O:
		{
			if (control)
				OpenScene();
			break;
		}
		case Key::S:
		{
			if (control && shift)
				SaveSceneAs();
			else if (control)
				SaveScene();
			break;
		}

			//Gizmos
		case Key::Q:
			m_GuizmoType = -1;
			break;
		case Key::W:
			m_GuizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case Key::E:
			m_GuizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case Key::R:
			m_GuizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}

		return true;
	}

	void EditorLayer::NewProject()
	{
		Project::New();
		NewScene();
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
			//AssetManager::SetProjectPath(std::filesystem::absolute(path).remove_filename());
			//AssetManager::ScanProject();

			assetManager.ScanDirectory(Project::GetAssetDirectory());

			if (Project::GetActive()->GetConfig().StartScene != "")
			{
				//NewScene();
				auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene).make_preferred();
				if (!OpenScene(startScenePath))
				{
					NewScene();
				}
			}
			else
			{
				NewScene();
			}
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

		m_SceneHierarchyPanel->SetContext(m_ActiveScene);
		m_ContentBrowserPanel->SetContext(m_ActiveScene);

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		m_CameraEntity.AddComponent<CameraComponent>();
	}

	bool EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Proton Scene (*.proton)\0*.proton\0");

		if (!filepath.empty())
		{
			return OpenScene(filepath);
		}

		return false;
	}

	bool EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (path.extension() != ".proton")
		{
			PT_CORE_ERROR("Could not open scene {0} - Not a Proton scene file", path.filename().string());
			return false;
		}

		if (!std::filesystem::exists(path))
		{
			PT_CORE_ERROR("Scene file {0} does not exist", path.string());
			return false;
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

			m_SceneHierarchyPanel->SetContext(m_ActiveScene);
			m_ContentBrowserPanel->SetContext(m_ActiveScene);

			m_CameraEntity = m_ActiveScene->FindEntityWithComponent<CameraComponent>();

			m_SceneRenderer->SetScene(m_ActiveScene);

			return true;
		}

		return false;
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

		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();

		m_SceneHierarchyPanel->SetContext(m_ActiveScene);
		m_SceneRenderer->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;

		m_ActiveScene->OnRuntimeStop();
		m_ActiveScene = m_EditorScene;

		m_SceneHierarchyPanel->SetContext(m_ActiveScene);
		m_SceneRenderer->SetScene(m_ActiveScene);
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit)
			return;

		m_ActiveScene->SetPaused(true);
	}
}