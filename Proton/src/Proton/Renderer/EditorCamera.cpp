#include "ptpch.h"
#include "EditorCamera.h"
#include "Proton/Core/Input.h"
#include "Proton/Core/KeyCodes.h"
#include "Proton/Core/Base.h"

#include "Proton\Core\Log.h"

#define RADIANS(x) x * 0.01745329f

namespace Proton
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		:
		m_FOV(fov),
		m_AspectRatio(aspectRatio),
		m_NearClip(nearClip),
		m_FarClip(farClip),
		Camera(XMMatrixPerspectiveFovLH(RADIANS(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::OnUpdate(TimeStep ts)
	{
		XMFLOAT2 mouseDelta = { Input::GetMouseDeltaX() * 0.025f,
							  Input::GetMouseDeltaY() * 0.025f };
		
		if (Input::IsMouseButtonPressed(2))
			MousePan(mouseDelta);
		else if (Input::IsMouseButtonPressed(0))
			MouseRotate(mouseDelta);
		else if (Input::IsMouseButtonPressed(1))
			MouseOrbit(mouseDelta);

		XMFLOAT3 translation = { 0, 0, 0 };

		float moveSpeed = m_MoveSpeed * ts;

		if (Input::IsKeyPressed(Key::W))
			translation.z += moveSpeed;
		if (Input::IsKeyPressed(Key::S))
			translation.z -= moveSpeed;
		if (Input::IsKeyPressed(Key::D))
			translation.x += moveSpeed;
		if (Input::IsKeyPressed(Key::A))
			translation.x -= moveSpeed;

		if (Input::IsKeyPressed(Key::E))
			translation.y += moveSpeed;
		if (Input::IsKeyPressed(Key::Q))
			translation.y -= moveSpeed;

		m_FocalPoint = CalculateFocalPoint(translation);

		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(PT_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
	}

	XMVECTOR EditorCamera::GetUpDirection() const
	{
		return XMVector3Rotate(XMVectorSet(0, 1, 0, 0), GetOrientation());
	}

	XMVECTOR EditorCamera::GetRightDirection() const
	{
		return XMVector3Rotate(XMVectorSet(1, 0, 0, 0), GetOrientation());
	}

	XMVECTOR EditorCamera::GetForwardDirection() const
	{
		return XMVector3Rotate(XMVectorSet(0, 0, 1, 0), GetOrientation());
	}

	XMVECTOR EditorCamera::GetOrientation() const
	{
		return XMQuaternionRotationRollPitchYaw(-m_Yaw, -m_Pitch, 0);
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_ProjectionMatrix = XMMatrixPerspectiveFovLH(RADIANS(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		m_Position = CalculatePosition();

		m_ViewMatrix = XMMatrixInverse(nullptr, XMMatrixRotationQuaternion(GetOrientation()) * 
												XMMatrixTranslationFromVector(m_Position));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = -e.GetWheelDelta() / 120.0f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const XMFLOAT2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const XMFLOAT2& delta)
	{
		float yawSign = XMVectorGetY(GetUpDirection()) < 0 ? -1.0f : 1.0f;
		m_Yaw -= yawSign * delta.y * RotationSpeed();
		m_Pitch -= delta.x * RotationSpeed();

		m_FocalPoint = m_Position + GetForwardDirection() * m_Distance;
	}

	void EditorCamera::MouseOrbit(const XMFLOAT2& delta)
	{
		float yawSign = XMVectorGetY(GetUpDirection()) < 0 ? -1.0f : 1.0f;
		m_Yaw -= yawSign * delta.y * RotationSpeed();
		m_Pitch -= delta.x * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance += delta * ZoomSpeed() * 5.0f;

		if (m_Distance < 1.0f)
			m_Distance = 1.0f;
	}

	XMVECTOR EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	XMVECTOR EditorCamera::CalculateFocalPoint(XMFLOAT3& translation) const
	{
		return XMVector3Transform(m_FocalPoint, 
			XMMatrixTranslationFromVector(XMVector3Rotate(XMLoadFloat3(&translation), GetOrientation())));
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min<float>(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min<float>(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.02f;
		return std::max<float>(std::min<float>(distance * distance, 1.0f), 0.03f);
	}

}

