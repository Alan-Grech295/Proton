#pragma once
#include <DirectXMath.h>

namespace Proton
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(DirectX::XMMATRIX& projection)
			:
			m_ProjectionMatrix(projection) {}

		virtual ~Camera() = default;

		const DirectX::XMMATRIX& GetProjection() const { return m_ProjectionMatrix; }
		void SetProjection(DirectX::XMMATRIX& projection) { m_ProjectionMatrix = projection; }

	protected:
		DirectX::XMMATRIX m_ProjectionMatrix = DirectX::XMMatrixIdentity();
	};
}