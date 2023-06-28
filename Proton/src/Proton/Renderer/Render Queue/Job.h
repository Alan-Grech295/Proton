#pragma once
#include "Proton/Model/StaticMesh.h"

namespace Proton
{
	class StaticMesh;
	class Material;
	class Material::Pass;

	class Job
	{
	public:
		Job(const StaticMesh* mesh, Ref<Material::Pass> step)
			:
			m_Mesh(mesh),
			m_MaterialPass(step)
		{}

	public:
		const StaticMesh* m_Mesh;
		Ref<Material::Pass> m_MaterialPass;
	};
}