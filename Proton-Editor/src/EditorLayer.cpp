#include "EditorLayer.h"
#include "CameraController.h"

namespace Proton
{
	EditorLayer::EditorLayer()
		:
		Layer("EditorLayer")
	{
		Application::Get().GetWindow().ShowCursor();
		//light = CreateRef<PointLight>(0.5f);

		m_ActiveScene = CreateRef<Scene>();
		sceneHierarchy.SetScene(m_ActiveScene);

		m_Nanosuit = Model::CreateModelEntity("C:\\Dev\\Proton\\Proton\\Models\\nano_textured\\nanosuit.obj", m_ActiveScene.get());
		m_GoblinEntity = Model::CreateModelEntity("C:\\Dev\\Proton\\Proton\\Models\\Goblin\\GoblinX.obj", m_ActiveScene.get());
		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		m_PointLight = m_ActiveScene->CreateEntity("Point Light");

		m_ActiveScene->framebuffer->SetClearCol(0.02f, 0.07f, 0.2f);

		//DirectX::XMMatrixPerspectiveLH(1.0f, (float)Application::Get().GetWindow().GetHeight() / Application::Get().GetWindow().GetWidth(), 0.5f, 1000.0f)

		m_CameraEntity.AddComponent<CameraComponent>();
		m_CameraEntity.AddScript<CameraController>();
		m_PointLight.AddComponent<LightComponent>("Point Light");
	}

	EditorLayer::~EditorLayer()
	{

	}

	void EditorLayer::OnUpdate(TimeStep ts)
	{
		PT_PROFILE_FUNCTION();

		if (Input::IsKeyReleased(PT_KEY_ESCAPE))
		{
			cursor = !cursor;
		}

		if (Input::IsKeyPressed(PT_KEY_ESCAPE))
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

		if (Input::IsKeyReleased(PT_KEY_0))
		{
			vSync = !vSync;
			Application::Get().GetWindow().SetVSync(vSync);
		}

		m_ActiveScene->OnUpdate(ts);
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
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		//Custom Windows
		if (ImGui::Begin("Debug Data"))
		{
			ImGui::Text("Application Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Scene");
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

		if (viewportPanelSize.x != m_ViewportSize.x || viewportPanelSize.y != m_ViewportSize.y)
		{
			m_ActiveScene->framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
			m_ViewportSize = viewportPanelSize;

			m_ActiveScene->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);

			CameraComponent& camera = m_CameraEntity.GetComponent< CameraComponent>();
			camera.camera.SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, (float)m_ViewportSize.y / m_ViewportSize.x, 0.5f, 1000.0f));
		}

		ImGui::PopStyleVar();

		ImGui::Image(m_ActiveScene->framebuffer->GetRenderTextureID(), viewportPanelSize);
		ImGui::End();

		sceneHierarchy.OnImGuiRender();
		//

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit", ""))
				{
					Application::Get().Close();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGui::End();
	}

	void EditorLayer::OnAttach()
	{

	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (e.IsEventType(EventType::KeyPressed))
		{
			KeyPressedEvent& event = (KeyPressedEvent&)e;
		}
	}
}