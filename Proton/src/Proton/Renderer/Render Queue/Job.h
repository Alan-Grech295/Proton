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
		Job(const Mesh* mesh, VertexConstantBuffer* vertConstBuf, PixelConstantBuffer* pixConstBuf, Ref<Material::Pass> step)
			:
			Mesh(mesh),
			MaterialPass(step),
			VertConstBuf(vertConstBuf),
			PixConstBuf(pixConstBuf)
		{}

	public:
		const Mesh* Mesh = nullptr;
		VertexConstantBuffer* VertConstBuf = nullptr;
		PixelConstantBuffer* PixConstBuf = nullptr;
		Ref<Material::Pass> MaterialPass;
	};
}