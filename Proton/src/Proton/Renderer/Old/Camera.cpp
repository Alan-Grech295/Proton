#include "ptpch.h"
#include "Camera.h"

namespace Proton::Depricated
{
	Camera::Camera(uint32_t screenWidth, uint32_t screenHeight, float nearZ, float farZ, ProjectionMode mode)
		:
		m_Position({0, 0, 0}),
		m_Rotation({0, 0, 0})
	{
		if (mode == ProjectionMode::Perspective)
		{
			m_ProjectionMatrix = DirectX::XMMatrixPerspectiveLH(1.0f, (float)screenHeight / screenWidth, nearZ, farZ);
		}
		else
		{
			m_ProjectionMatrix = DirectX::XMMatrixOrthographicLH(10, (float)screenHeight / screenWidth * 10, nearZ, farZ);
		}

		RecalculateViewMatrix();
	}

	void Camera::RecalculateViewMatrix()
	{
		m_ViewMatrix = DirectX::XMMatrixInverse(nullptr,
					   DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) *
					   DirectX::XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z)
					   );
					   
		m_ViewProjectionMatrix = m_ViewMatrix * m_ProjectionMatrix;
	}

	const DirectX::XMMATRIX Camera::GetRotationMatrix() const
	{
		DirectX::XMFLOAT3X3 rotMat;
		DirectX::XMStoreFloat3x3(&rotMat, m_ViewMatrix);

		return DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat3x3(&rotMat));
	}
}