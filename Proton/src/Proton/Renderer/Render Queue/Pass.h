#pragma once
#include <string>
#include "Job.h"
#undef AddJob

namespace Proton
{
	class Pass
	{
	public:
		Pass(const std::string& name, int passID)
			: m_Name(name), m_PassID(passID)
		{}

		Pass()
			:
			m_Name("")
		{}

		void AddJob(Ref<Job> job)
		{
			m_Jobs.push_back(std::move(job));
		}

		void Clear()
		{
			m_Jobs.clear();
		}

		std::vector<Ref<Job>>::iterator begin()
		{
			return m_Jobs.begin();
		}

		std::vector<Ref<Job>>::iterator end()
		{
			return m_Jobs.end();
		}
	public:
		std::string m_Name;
		int m_PassID = -1;
	private:
		std::vector<Ref<Job>> m_Jobs;
	};
}