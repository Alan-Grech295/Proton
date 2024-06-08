#include "ptpch.h"
#include "Mesh.h"

namespace Proton
{
	Mesh::Mesh(UUID assetID)
		: m_AssetID(assetID)
	{
		m_Topology = Topology::Create(TopologyType::TriangleList);
	}
}

