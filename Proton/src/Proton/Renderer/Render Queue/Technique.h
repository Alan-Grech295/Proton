#pragma once
#include "Step.h"

namespace Proton
{
	class Technique
	{
	public:
		Technique()
			:
			m_Name("")
		{}

		Technique(const std::string& name)
			:
			m_Name(name)
		{}
		void AddStep(Step step)
		{
			m_Steps.push_back(std::move(step));
		}

		std::vector<Step>::iterator begin()
		{
			return m_Steps.begin();
		}

		std::vector<Step>::const_iterator cbegin() 
		{
			return m_Steps.cbegin();
		}

		std::vector<Step>::iterator end()
		{
			return m_Steps.end();
		}

		const std::vector<Step>::const_iterator cend()
		{
			return m_Steps.cend();
		}

	public:
		std::string m_Name;
	private:
		std::vector<Step> m_Steps;
	};
}