#pragma once
#include "Camera.h"
#include "Proton\Core\TimeStep.h"
#include "Proton\Events\Event.h"
#include "Proton\Events\MouseEvent.h"

using namespace DirectX;

namespace Proton
{
	class EditorCamera : public Camera
	{
		friend class SceneSerializer;
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate(TimeStep ts);
		void OnEvent(Event & e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		const XMMATRIX& GetViewMatrix() const { return m_ViewMatrix; }
		XMMATRIX& GetViewProjection() const { return m_ViewMatrix * m_ProjectionMatrix; }

		XMVECTOR GetUpDirection() const;
		XMVECTOR GetRightDirection() const;
		XMVECTOR GetForwardDirection() const;
		const XMVECTOR& GetPosition() const { return m_Position; }
		XMVECTOR GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const XMFLOAT2& delta);
		void MouseRotate(const XMFLOAT2& delta);
		void MouseOrbit(const XMFLOAT2& delta);
		void MouseZoom(float delta);

		XMVECTOR CalculatePosition() const;
		
		XMVECTOR CalculateFocalPoint(XMFLOAT3& translation) const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	public:
		//TEMP
		//float zoomReducDist = 50.0f;
		//float zoomSpeed = 3.5f;
		//float rotSpeed = 0.8f;
	private:
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;

		XMMATRIX m_ViewMatrix;
		XMVECTOR m_Position = { 0.0f, 0.0f, 0.0f, 0.0f };
		XMVECTOR m_FocalPoint = { 0.0f, 0.0f, 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_MoveSpeed = 100.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}