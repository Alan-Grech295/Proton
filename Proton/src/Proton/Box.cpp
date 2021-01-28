#include "ptpch.h"
#include "Proton/Box.h"
#include "Proton/Log.h"
#include "Proton/Cube.h"

//Temp
#include "Platform\Windows\WindowsGraphics.h"

namespace Proton
{
	Box::Box(std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		std::uniform_real_distribution<float>& bdist,
		DirectX::XMFLOAT3 material)
		:
		r(rdist(rng)),
		droll(ddist(rng)),
		dpitch(ddist(rng)),
		dyaw(ddist(rng)),
		dphi(odist(rng)),
		dtheta(odist(rng)),
		dchi(odist(rng)),
		chi(adist(rng)),
		theta(adist(rng)),
		phi(adist(rng))
	{
		namespace dx = DirectX;
		
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
		};

		auto model = Cube::MakeIndependent<Vertex>();
		model.SetNormalsIndependentFlat();

		m_VertBuffer.reset(VertexBuffer::Create(sizeof(Vertex), &model.vertices[0], model.vertices.size()));

		m_VertShader.reset(VertexShader::Create(WindowsGraphics::GetShaderPath("PhongVS.cso")));

		m_PixelShader.reset(PixelShader::Create(WindowsGraphics::GetShaderPath("PhongPS.cso")));

		m_IndexBuffer.reset(IndexBuffer::Create(&model.indices[0], model.indices.size()));

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3}
		};

		m_VertBuffer->SetLayout(layout, m_VertShader.get());

		m_TransformCBuf.reset(VertexConstantBuffer::Create());

		materialConstants.color = material;
		m_MaterialCBuf.reset(PixelConstantBuffer::Create(0, sizeof(materialConstants), &materialConstants));

		// model deformation transform (per instance, not stored as bind)
		dx::XMStoreFloat3x3(
			&mt,
			dx::XMMatrixScaling(1.0f, 1.0f, bdist(rng))
		);
	}

	void Box::Update(float dt) noexcept
	{
		roll += droll * dt;
		pitch += dpitch * dt;
		yaw += dyaw * dt;
		theta += dtheta * dt;
		phi += dphi * dt;
		chi += dchi * dt;
	}

	DirectX::XMMATRIX Box::GetTransformXM() const noexcept
	{
		namespace dx = DirectX;
		return dx::XMLoadFloat3x3(&mt) *
			dx::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
			dx::XMMatrixTranslation(r, 0.0f, 0.0f) *
			dx::XMMatrixRotationRollPitchYaw(theta, phi, chi);
	}
}