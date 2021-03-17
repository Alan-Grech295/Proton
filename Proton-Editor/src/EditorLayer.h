#pragma once
#define NOMINMAX
#include <Proton.h>
#include "imgui\imgui.h"
#include "Panels\SceneHierarchyPanel.h"

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

		void MoveCamera(TimeStep ts);
	private:
		Entity m_CameraEntity;
		Entity m_GoblinEntity;
		Entity m_Nanosuit;
		Entity m_PointLight;

		Ref<Scene> m_ActiveScene;

		ImVec2 m_ViewportSize;		

		SceneHierarchyPanel sceneHierarchy;

		float cameraSpeed = 15.0f;
		float rotationSpeed = 0.8f;
		bool enableCamUpdate = true;
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