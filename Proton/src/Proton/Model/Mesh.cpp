#include "ptpch.h"
#include "Mesh.h"

namespace Proton
{
	Mesh::Mesh(Ref<class Model> model)
	{
		m_Topology = Topology::Create(TopologyType::TriangleList);

		m_Model = model;
	}
}

