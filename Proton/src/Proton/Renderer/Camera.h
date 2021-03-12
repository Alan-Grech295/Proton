#pragma once
#include <DirectXMath.h>

namespace Proton
{
	class Camera
	{
	public:
		Camera(DirectX::FXMMATRIX& projection)
			:
			m_ProjectionMatrix(projection) {}

		const DirectX::XMMATRIX& GetProjection() const { return m_ProjectionMatrix; }
		void SetProjection(DirectX::FXMMATRIX& projection) { m_ProjectionMatrix = projection; }
	private:
		DirectX::XMMATRIX m_ProjectionMatrix;
	};
}