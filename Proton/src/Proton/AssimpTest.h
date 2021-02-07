#pragma once
#include "Math.h"
#include <random>
#include "Renderer\Buffer.h"
#include "Renderer\Shader.h"
#include <DirectXMath.h>

namespace Proton
{
	class AssimpTest
	{
	public:
		AssimpTest(std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist,
			DirectX::XMFLOAT3 material);

		void Update(float dt) noexcept;

		DirectX::XMMATRIX GetTransformXM() const noexcept
		{
			namespace dx = DirectX;
			return dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
				   dx::XMMatrixTranslation(r, 0.0f, 0.0f) *
				   dx::XMMatrixRotationRollPitchYaw(theta, phi, chi);
		}
	private:
		// positional
		float r;
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float theta;
		float phi;
		float chi;
		// speed (delta/s)
		float droll;
		float dpitch;
		float dyaw;
		float dtheta;
		float dphi;
		float dchi;

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};

		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color = { 0, 0, 0 };
			float specularIntensity = 2.0f;
			float specularPower = 30.0f;
			float padding[3] = { 0, 0, 0 };
		} pmc;
	public:
		std::unique_ptr<VertexBuffer> m_VertBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<VertexShader> m_VertShader;
		std::unique_ptr<PixelShader> m_PixelShader;
		std::unique_ptr<VertexConstantBuffer> m_TransformCBuf;
		std::unique_ptr<PixelConstantBuffer> m_MaterialCBuf;
	};
}