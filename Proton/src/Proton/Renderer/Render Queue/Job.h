#pragma once
#include "Step.h"

namespace Proton
{
	class Mesh;
	class Job
	{
	public:
		Job(Mesh* mesh, Step* step)
			:
			m_Mesh(mesh),
			m_Step(step)
		{}

	public:
		Mesh* m_Mesh;
		Step* m_Step;
	};
}