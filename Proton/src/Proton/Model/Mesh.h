#pragma once
#include "Material.h"
#include "Proton/Core/UUID.h"

namespace Proton
{
	class Mesh
	{
	public:
		Mesh(UUID assetID);

		Mesh() = default;
	public:
		std::string m_Name;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Topology> m_Topology;

		Ref<Material> m_DefaultMaterial;

		UUID m_AssetID;
	};
}


