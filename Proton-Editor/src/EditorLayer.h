#pragma once
#define NOMINMAX
#include <Proton.h>
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "Panels\SceneHierarchyPanel.h"
#include "Panels\ContentBrowserPanel.h"
#include "Panels\ConsolePanel.h"
#include "EditorSceneRenderer/EditorSceneRenderer.h"
#include "Proton\Renderer\EditorCamera.h"
#include "Proton/Asset System/Editor/EditorAssetManager.h"

#include "ImGuizmo.h"

namespace Proton
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		~EditorLayer();

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnAttach() override;
		virtual void OnEvent(Event& e) override;
	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void NewProject();
		void OpenProject();
		void OpenProject(const std::filesystem::path& path);
		void SaveProject();

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveScene();
		void SaveSceneAs();

		void UI_Toolbar();

		void OnScenePlay();
		void OnSceneStop();
		void OnScenePause();
	private:
		Entity m_CameraEntity;
		Entity m_GoblinEntity;
		Entity m_Nanosuit;
		Entity m_PointLight;

		std::string projectPath;

		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;

		Scope<ContentBrowserPanel> m_ContentBrowserPanel;
		Scope<ConsolePanel> m_ConsolePanel;
		Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;

		Scope<EditorSceneRenderer> m_SceneRenderer;

		Ref<Texture2D> m_IconPlay, m_IconStop, m_IconPause, m_IconStep;

		EditorCamera m_EditorCam;

		EditorAssetManager assetManager;

		ImVec2 m_ViewportSize;		
		ImVec2 m_ViewportBounds[2];

		std::string saveFilePath;

		uint32_t m_AntiAliasing = 1;

		bool cursor = true;

		// Dragging check
		const float dragThreshold = 5;
		float mouseClickX;
		float mouseClickY;
		bool dragging = false;
		bool initClick = true;

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		bool m_UpdateEditorCamera = true;

		int m_GuizmoType = ImGuizmo::OPERATION::TRANSLATE;

		struct
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		} m_Transform;

		enum class SceneState { Edit = 0, Play = 1, Simulate = 2 };

		SceneState m_SceneState = SceneState::Edit;
	};
}

#define LOG_ERROR(x) m_ConsolePanel->LogError(x)
#define LOG_WARNING(x) m_ConsolePanel->LogWarning(x)
#define LOG_TRACE(x) m_ConsolePanel->LogTrace(x)