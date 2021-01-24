#pragma once
#include "Platform/Windows/WindowsGraphics.h"
#include "SolidSphere.h"
#include "ConstantBuffer.h"

namespace Proton
{
	class PointLight
	{
	public:
		PointLight(WindowsGraphics& gfx, float radius = 0.5f);
		void CreateControlWindow();
		void Reset();
		void Draw(WindowsGraphics& gfx) const;
		void Bind(WindowsGraphics& gfx) const;
	private:
		struct PointLightCBuf
		{
			alignas(16) DirectX::XMFLOAT3 pos;
			alignas(16) DirectX::XMFLOAT3 materialColor;
			alignas(16) DirectX::XMFLOAT3 ambient;
			alignas(16) DirectX::XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		};
	private:
		PointLightCBuf cbData;
		mutable SolidSphere mesh;
		mutable PixelConstantBuffer<PointLightCBuf> cbuf;
	};
}