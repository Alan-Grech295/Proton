#pragma once
#include "Platform/Windows/WindowsGraphics.h"

namespace Proton
{
	class Camera
	{
	public:
		DirectX::XMMATRIX GetMatrix() const;
		void CreateControlWindow();
		void Reset();
	private:
		float r = 20.0f;
		float theta = 0.0f;
		float phi = 0.0;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float roll = 0.0f;
	};
}