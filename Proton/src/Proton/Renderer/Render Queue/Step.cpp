#include "ptpch.h"
#include "Step.h"
#include "../Renderer.h"

namespace Proton
{
	Step::Step(const std::string& passName)
		:
		m_PassID(Renderer::GetPassIDFromName(passName))
	{}
}
