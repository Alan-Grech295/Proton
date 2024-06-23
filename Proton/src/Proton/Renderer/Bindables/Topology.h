#pragma once
#include "Bindable.h"
#include <string>
#include <assert.h>

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

		virtual Ref<Bindable> Clone() override
		{
			return Clone(*this);
		}

		TopologyType GetTopology() { return m_Topology; }

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

		static Scope<Topology> CreateUnique(Ref<Bindable> other)
		{
			assert("No need for unique topology" && false);
			return CreateScope<Topology>(TopologyType::None);
		}

		static Ref<Topology> Clone(const Topology& other)
		{
			return Create(other.m_Topology);
		}

	private:
		TopologyType m_Topology;
	};
}