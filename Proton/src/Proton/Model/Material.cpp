#include "ptpch.h"
#include "Material.h"
#include "Proton/Renderer/Renderer.h"

namespace Proton
{
	Material::Material(const std::string& passName, const std::string& name, UUID assetID)
		: assetID(assetID), m_Name(name)
	{
		m_PassID = Renderer::GetPassIDFromName(passName);
	}

	Material::Material(int passID, const std::string& name, UUID assetID)
		: m_PassID(passID), assetID(assetID), m_Name(name)
	{
	}

	Material::Material(const std::string& passName)
		: assetID(UUID::Null), m_Name("")
	{
		m_PassID = Renderer::GetPassIDFromName(passName);
	}

	Material::Material()
		: assetID(UUID::Null), m_Name(""), m_PassID(-1)
	{
	}

	Ref<Material> Material::Clone()
	{
		Ref<Material> clone = CreateRef<Material>(m_PassID, m_Name, assetID);

		clone->m_VertexShader = m_VertexShader;
		clone->m_PixelShader = m_PixelShader;

		clone->hasSpecular = hasSpecular;
		clone->hasNormalMap = hasNormalMap;
		clone->hasDiffuseMap = hasDiffuseMap;

		clone->m_Bindables.reserve(m_Bindables.size());
		for (Ref<Bindable> bind : m_Bindables)
		{
			clone->m_Bindables.push_back(bind->Clone());
		}

		return clone;
	}
}