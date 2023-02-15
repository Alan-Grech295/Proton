#pragma once
#define NOMINMAX
#include <Proton.h>
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "Panels\SceneHierarchyPanel.h"
#include "Panels\AssetViewerPanel.h"
#include "Panels\ConsolePanel.h"
#include <Proton\Renderer\SceneRenderer.h>
#include "Proton\Renderer\EditorCamera.h"

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

		void DrawScope(const ScopeNode* scope, uint32_t scopeLevel);
	private:
		bool OnKeyPressed(KeyPressedEvent& e);

		void NewScene();
		void OpenScene();
		void SaveScene();
		void SaveSceneAs();

		void UI_Toolbar();

		void OnScenePlay();
		void OnSceneStop();
	private:
		Entity m_CameraEntity;
		Entity m_GoblinEntity;
		Entity m_Nanosuit;
		Entity m_PointLight;

		std::string projectPath = CORE_PATH_STR + "\\Proton-Editor\\assets";

		Ref<Scene> m_ActiveScene;
		Scope<SceneRenderer> m_SceneRenderer;

		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;

		EditorCamera m_EditorCam;

		bool m_UpdateEditorCam = true;

		ImVec2 m_ViewportSize;		

		std::string saveFilePath;

		bool cursor = true;

		struct
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		} m_Transform;

		enum class SceneState { Edit = 0, Play = 1};

		SceneState m_SceneState = SceneState::Edit;
	};
}

#define LOG_ERROR(x) Proton::ConsolePanel::LogError(x)
#define LOG_WARNING(x) Proton::ConsolePanel::LogWarning(x)
#define LOG_TRACE(x) Proton::ConsolePanel::LogTrace(x)