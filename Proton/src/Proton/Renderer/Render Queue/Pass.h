#pragma once
#include <string>
#include "Job.h"
#undef AddJob

namespace Proton
{
	class Pass
	{
	public:
		Pass(const std::string& name)
			:
			m_Name(name)
		{}

		Pass()
			:
			m_Name("")
		{}

		void AddJob(Job job)
		{
			m_Jobs.push_back(std::move(job));
		}

		void Clear()
		{
			m_Jobs.clear();
		}

		std::vector<Job>::iterator begin()
		{
			return m_Jobs.begin();
		}

		std::vector<Job>::iterator end()
		{
			return m_Jobs.end();
		}
	public:
		std::string m_Name;
	private:
		std::vector<Job> m_Jobs;
	};
}