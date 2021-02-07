#pragma once
#include <random>
#include <DirectXMath.h>
#include "Renderer\Buffer.h"
#include "Renderer\Shader.h"

namespace Proton
{
	class Box
	{
	public:
		Box(std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist,
			std::uniform_real_distribution<float>& bdist,
			DirectX::XMFLOAT3 material);
		void Update(float dt) noexcept;
		DirectX::XMMATRIX GetTransformXM() const noexcept;
	private:
		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color = { 1, 1, 1 };
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[3] = { 0, 0, 0 };
		} materialConstants;
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
		// model transform
		DirectX::XMFLOAT3X3 mt;

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};

	public:
		std::unique_ptr<VertexBuffer> m_VertBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<VertexShader> m_VertShader;
		std::unique_ptr<PixelShader> m_PixelShader;
		std::unique_ptr<VertexConstantBuffer> m_TransformCBuf;
		std::unique_ptr<PixelConstantBuffer> m_MaterialCBuf;
	};
}