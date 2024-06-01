#pragma once
#include "Proton/Model/Mesh.h"

namespace Proton
{
	class Mesh;
	class Material;
	class Material::Pass;

	class Job
	{
	public:
		Job(const Mesh* mesh, Ref<Material::Pass> step)
			:
			m_Mesh(mesh),
			m_MaterialPass(step)
		{}

	public:
		const Mesh* m_Mesh;
		Ref<Material::Pass> m_MaterialPass;
	};
}