#include "ptpch.h"
#include "Topology.h"
#include "Proton\Renderer\Renderer.h"
#include <cassert>
#include "BindsCollection.h"

namespace Proton
{
	Ref<Topology> Topology::Create(TopologyType type)
	{
		PT_PROFILE_FUNCTION();

		return BindsCollection::Resolve<Topology>(type);
	}

	Scope<Topology> Topology::CreateUnique(TopologyType type)
	{
		PT_PROFILE_FUNCTION();

		return CreateScope<Topology>(type);
	}

	void Topology::Bind()
	{
		RenderCommand::SetTopology(m_Topology);
	}
}
