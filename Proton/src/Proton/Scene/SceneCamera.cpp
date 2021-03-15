#include "ptpch.h"
#include "SceneCamera.h"

namespace Proton
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	SceneCamera::~SceneCamera()
	{
	}

	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;

		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetPerspective(float fov, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;

		m_PerspectiveFOV = fov;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_ProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_PerspectiveFOV * 0.01745329f, m_AspectRatio,
																   m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthoHorizontal = m_OrthographicSize * m_AspectRatio;
			float orthoVertical = m_OrthographicSize;

			m_ProjectionMatrix = DirectX::XMMatrixOrthographicLH(orthoHorizontal, orthoVertical,
																 m_OrthographicNear, m_OrthographicFar);
		}
	}
}
