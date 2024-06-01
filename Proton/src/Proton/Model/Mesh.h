#pragma once
#include "Proton/Renderer/Bindables/Binds.h"
#include "Material.h"

namespace Proton
{
	class Mesh
	{
		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	public:
		Mesh() 
		{
			m_TransformBufferVert = VertexConstantBuffer::CreateUnique(0, sizeof(Transforms), new Transforms());
			DCB::RawLayout layout;
			layout.Add(DCB::Type::Matrix4x4, "modelViewProj");
			layout.Add(DCB::Type::Matrix4x4, "model");
			m_TransformBufferPix = PixelConstantBuffer::CreateUnique(2, DCB::CookedLayout(std::move(layout)));
			m_Topology = Topology::Create(TopologyType::TriangleList);
		}
	public:
		std::string m_Name;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Topology> m_Topology;

		Scope<VertexConstantBuffer> m_TransformBufferVert;
		Scope<PixelConstantBuffer> m_TransformBufferPix;

		Ref<Material> material;
	};
}


