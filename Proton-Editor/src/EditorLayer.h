#pragma once
#define NOMINMAX
#include <Proton.h>
#include "imgui\imgui.h"
#include "Panels\SceneHierarchyPanel.h"
#include "Panels\AssetViewerPanel.h"
#include "Panels\ConsolePanel.h"
#include <Proton\Renderer\SceneRenderer.h>
#include <Proton\Renderer\EditorCamera.h>
#include <Proton\Asset Loader\AssetManager.h>
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
	private:
		Entity m_CameraEntity;
		Entity m_GoblinEntity;
		Entity m_Nanosuit;
		Entity m_PointLight;

		Ref<Scene> m_ActiveScene;
		Scope<SceneRenderer> m_SceneRenderer;

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
	};
}

#define LOG_ERROR(x) Proton::ConsolePanel::LogError(x)
#define LOG_WARNING(x) Proton::ConsolePanel::LogWarning(x)
#define LOG_TRACE(x) Proton::ConsolePanel::LogTrace(x)