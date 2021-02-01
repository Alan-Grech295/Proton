#include "ptpch.h"
#include "Camera.h"

namespace Proton
{
	Camera::Camera(uint32_t screenWidth, uint32_t screenHeight, float nearZ, float farZ, ProjectionMode mode)
		:
		m_Position({0, 0, 20}),
		m_Rotation({0, 0, 0})
	{
		if (mode == ProjectionMode::Perspective)
		{
			m_ProjectionMatrix = DirectX::XMMatrixPerspectiveLH(1.0f, (float)screenHeight / screenWidth, nearZ, farZ);
		}
		else
		{
			m_ProjectionMatrix = DirectX::XMMatrixOrthographicLH(1.0f, (float)screenHeight / screenWidth, nearZ, farZ);
		}

		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;

		m_ViewMatrix = DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z) *
			DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	}

	void Camera::RecalculateViewMatrix()
	{
		m_ViewMatrix = DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z) *
					   DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}
}