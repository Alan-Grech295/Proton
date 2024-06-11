#pragma once
#include "Proton/Model/Mesh.h"

namespace Proton
{
	class Mesh;
	class Material;

	class Job
	{
	public:
		Job(const Mesh* mesh, VertexConstantBuffer* vertConstBuf, PixelConstantBuffer* pixConstBuf, Ref<Material> step)
			:
			Mesh(mesh),
			Material(step),
			VertConstBuf(vertConstBuf),
			PixConstBuf(pixConstBuf)
		{}

	public:
		const Mesh* Mesh = nullptr;
		VertexConstantBuffer* VertConstBuf = nullptr;
		PixelConstantBuffer* PixConstBuf = nullptr;
		Ref<Material> Material;
	};
}