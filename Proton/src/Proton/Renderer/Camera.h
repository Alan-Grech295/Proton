#pragma once
#include <DirectXMath.h>
#include "Proton\Core.h"

namespace Proton
{
	class Camera
	{
	public:
		enum class ProjectionMode { Perspective, Orthographic };
	public:
		Camera(uint32_t screenWidth, uint32_t screenHeight, float nearZ, float farZ, ProjectionMode mode);

		const DirectX::XMFLOAT3 GetPosition() const { return m_Position; }
		void SetPosition(DirectX::XMFLOAT3 position) { m_Position = position; RecalculateViewMatrix();}

		const DirectX::XMFLOAT3 GetRotation() const { return m_Rotation; };
		void SetRotation(DirectX::XMFLOAT3 rotation) { m_Rotation = rotation; RecalculateViewMatrix();}

		const DirectX::XMMATRIX GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const DirectX::XMMATRIX GetViewMatrix() const { return m_ViewMatrix; }
		const DirectX::XMMATRIX GetRotationMatrix() const;
		const DirectX::XMMATRIX GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
		void RecalculateViewMatrix();
	private:
		DirectX::XMMATRIX m_ProjectionMatrix;
		DirectX::XMMATRIX m_ViewMatrix;
		DirectX::XMMATRIX m_ViewProjectionMatrix;

		DirectX::XMFLOAT3 m_Position;
		DirectX::XMFLOAT3 m_Rotation;
	};
}