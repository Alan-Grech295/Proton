#pragma once
#include "Material.h"

namespace Proton
{
	class Mesh
	{
	public:
		Mesh(Ref<class Model> model);

		Mesh() = default;
	public:
		std::string m_Name;

		Ref<VertexBuffer> m_VertexBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Topology> m_Topology;

		Ref<Material> material;

		WeakRef<class Model> m_Model;
	};
}


