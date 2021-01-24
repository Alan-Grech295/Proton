#include "ptpch.h"
#include "Proton/Box.h"
#include "Proton/BindableBase.h"
#include "Proton/Log.h"
#include "Proton/Cube.h"

namespace Proton
{
	Box::Box(WindowsGraphics& gfx,
		std::mt19937& rng,
		std::uniform_real_distribution<float>& adist,
		std::uniform_real_distribution<float>& ddist,
		std::uniform_real_distribution<float>& odist,
		std::uniform_real_distribution<float>& rdist,
		std::uniform_real_distribution<float>& bdist)
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
		if (!IsStaticInitialised())
		{
			struct Vertex
			{
				dx::XMFLOAT3 pos;
				dx::XMFLOAT3 n;
			};
			
			auto model = Cube::MakeIndependent<Vertex>();
			model.SetNormalsIndependentFlat();

			AddStaticBind(std::make_unique<VertexBuffer>(gfx, model.vertices));

			auto pvs = std::make_unique<VertexShader>(gfx, gfx.GetShaderPath("PhongVS.cso"));
			auto pvsbc = pvs->GetBytecode();
			AddStaticBind(std::move(pvs));

			AddStaticBind(std::make_unique<PixelShader>(gfx, gfx.GetShaderPath("PhongPS.cso")));

			AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, model.indices));

			const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
			{
				{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
				{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};
			AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

			AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
		}
		else
		{
			SetIndexFromStatic();
		}

		AddBind(std::make_unique<TransformCBuf>(gfx, *this));

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