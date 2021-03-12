#pragma once
#define NOMINMAX
#include <Proton.h>
#include "imgui\imgui.h"

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
		Entity m_CameraEntity;
		Entity m_GoblinEntity;
		Ref<PointLight> light;

		Ref<Scene> m_ActiveScene;

		CameraComponent& cameraProjection;

		ImVec2 m_ViewportSize;

		DirectX::XMFLOAT3 m_CameraPos{ 0, 0, -20 };
		float cameraSpeed = 15.0f;
		float rotationSpeed = 0.8f;

		bool enableCursor = true;
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