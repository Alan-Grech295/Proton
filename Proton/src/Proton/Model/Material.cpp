#include "ptpch.h"
#include "Material.h"
#include "Proton/Renderer/Renderer.h"

namespace Proton
{
	Material::Pass::Pass(const std::string& passName)
	{
		m_PassID = Renderer::GetPassIDFromName(passName);
	}
}