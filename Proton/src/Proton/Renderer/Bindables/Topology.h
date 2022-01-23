#pragma once
#include "Bindable.h"
#include <string>

namespace Proton
{
	enum class TopologyType
	{
		None = 0, TriangleList, LineList, PointList
	};

	class Topology : public Bindable
	{
	public:
		Topology(TopologyType type)
			:
			m_Topology(type)
		{}

		~Topology() {}

		virtual void Bind() override;

		std::string GetUID() const noexcept { return GenerateUID(m_Topology); }

		static std::string GenerateUID(TopologyType type) 
		{ 
			switch (type)
			{
				case TopologyType::TriangleList: return "TriangleList";
				case TopologyType::LineList: return "LineList";
				case TopologyType::PointList: return "PointList";
			}

			return "None";
		}

		static Ref<Topology> Create(TopologyType type);

		static Scope<Topology> CreateUnique(TopologyType type);

	private:
		TopologyType m_Topology;
	};
}