#pragma once
#include "SolidSphere.h"
#include "Renderer\Buffer.h"

namespace Proton
{
	class PointLight
	{
	public:
		PointLight(float radius = 0.5f);
		void CreateControlWindow();
		void Reset();
		void SetLightData() const;
	public:
		mutable SolidSphere mesh;
	private:
		struct PointLightCBuf
		{
			alignas(16) DirectX::XMFLOAT3 pos;
			alignas(16) DirectX::XMFLOAT3 ambient;
			alignas(16) DirectX::XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		};
	private:
		PointLightCBuf cbData;
		mutable std::unique_ptr<PixelConstantBuffer> cbuf;
	};
}