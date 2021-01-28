#include "ptpch.h"
#include "SolidSphere.h"
#include "Sphere.h"
#include "Platform\Windows\WindowsGraphics.h"

namespace Proton
{
	SolidSphere::SolidSphere(float radius)
	{
		namespace dx = DirectX;
		
		struct Vertex
		{
			dx::XMFLOAT3 pos;
		};

		auto model = Sphere::Make<Vertex>();
		model.Transform(dx::XMMatrixScaling(radius, radius, radius));

		m_VertBuffer.reset(VertexBuffer::Create(sizeof(Vertex), &model.vertices[0], model.vertices.size()));

		m_IndexBuffer.reset(IndexBuffer::Create(&model.indices[0], model.indices.size()));

		m_VertShader.reset(VertexShader::Create(WindowsGraphics::GetShaderPath("SolidVS.cso")));

		m_PixelShader.reset(PixelShader::Create(WindowsGraphics::GetShaderPath("SolidPS.cso")));

		m_MaterialCBuf.reset(PixelConstantBuffer::Create(0, sizeof(colorConst), &colorConst));

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3}
		};

		m_VertBuffer->SetLayout(layout, m_VertShader.get());

		m_TransformCBuf.reset(VertexConstantBuffer::Create());
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
}