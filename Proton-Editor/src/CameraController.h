#pragma once
#include <Proton.h>

namespace Proton
{
	class CameraController : public ScriptableEntity
	{
	public:
		void OnUpdate(TimeStep ts)
		{
			float speed = ts * cameraSpeed;

			//Temp
			TransformComponent& cameraTransform = GetComponent<TransformComponent>();

			DirectX::XMFLOAT3 localMove = { 0, 0, 0 };

			if (Input::IsKeyPressed(PT_KEY_D))
				localMove.x += speed;

			if (Input::IsKeyPressed(PT_KEY_A))
				localMove.x -= speed;

			if (Input::IsKeyPressed(PT_KEY_E))
				localMove.y += speed;

			if (Input::IsKeyPressed(PT_KEY_Q))
				localMove.y -= speed;

			if (Input::IsKeyPressed(PT_KEY_W))
				localMove.z += speed;

			if (Input::IsKeyPressed(PT_KEY_S))
				localMove.z -= speed;

			if (Input::IsKeyReleased(PT_KEY_SPACE))
			{
				enableCursor = !enableCursor;
			}

			if (enableCursor)
			{
				cameraTransform.rotation.x += rotationSpeed * Input::GetMouseDeltaY() * std::max<float>(0.001f, ts);
				cameraTransform.rotation.y += rotationSpeed * Input::GetMouseDeltaX() * std::max<float>(0.001f, ts);
			}

			DirectX::XMStoreFloat3(&localMove, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&localMove), DirectX::XMMatrixRotationRollPitchYaw(cameraTransform.rotation.x, cameraTransform.rotation.y, cameraTransform.rotation.z)));

			cameraTransform.position.x += localMove.x;
			cameraTransform.position.y += localMove.y;
			cameraTransform.position.z += localMove.z;
		}
	private:
		float cameraSpeed = 15.0f;
		float rotationSpeed = 0.8f;
		bool enableCursor = true;

	};
}