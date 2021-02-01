#include "ptpch.h"
#include "SolidSphere.h"
#include "Sphere.h"
#include "Renderer\Renderer.h"

namespace Proton
{
	SolidSphere::SolidSphere(float radius)
		:
		pos({0, 0, 0})
	{
		namespace dx = DirectX;
		
		struct Vertex
		{
			dx::XMFLOAT3 pos;
		};

		auto model = Sphere::Make<Vertex>();
		model.Transform(dx::XMMatrixScaling(radius, radius, radius));

		m_VertBuffer.reset(VertexBuffer::Create(sizeof(Vertex), model.vertices.data(), model.vertices.size()));

		m_IndexBuffer.reset(IndexBuffer::Create(model.indices.data(), model.indices.size()));

		m_VertShader.reset(VertexShader::Create("C:\\Dev\\Proton\\Proton\\SolidVS.cso"));

		m_PixelShader.reset(PixelShader::Create("C:\\Dev\\Proton\\Proton\\SolidPS.cso"));

		m_MaterialCBuf.reset(PixelConstantBuffer::Create(0, sizeof(colorConst), &colorConst));

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3}
		};

		m_VertBuffer->SetLayout(layout, m_VertShader.get());

		m_TransformCBuf.reset(VertexConstantBuffer::Create(0, sizeof(Transforms), new Transforms()));
	}

	void SolidSphere::Update(float dt) noexcept {}

	void SolidSphere::SetPos(DirectX::XMFLOAT3 pos)
	{
		this->pos = pos;
	}

	DirectX::XMMATRIX SolidSphere::GetTransformXM() const noexcept
	{
		return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	}

	void SolidSphere::Bind()
	{
		const auto modelView = GetTransformXM() * Renderer::GetCamera().GetViewMatrix();
		const Transforms tf =
		{
			DirectX::XMMatrixTranspose(modelView),
			DirectX::XMMatrixTranspose(
				modelView *
				Renderer::GetCamera().GetProjectionMatrix()
			)
		};

		m_TransformCBuf->SetData(sizeof(Transforms), &tf);

		m_VertShader->Bind();
		m_PixelShader->Bind();
		m_TransformCBuf->Bind();
		m_MaterialCBuf->Bind();
	}
}