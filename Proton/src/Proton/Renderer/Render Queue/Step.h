#pragma once
#include <vector>
#include "../Bindables/Bindable.h"

namespace Proton
{
	class Step
	{
	public:
		Step(int passID)
			:
			m_PassID(passID)
		{}

		Step(const std::string& passName);

		Step()
			:
			m_PassID(-1)
		{}

		void AddBindable(Ref<Bindable> bindable)
		{
			m_Bindables.push_back(std::move(bindable));
		}

		void AddBindable(Scope<Bindable> bindable)
		{
			m_Bindables.push_back(std::move(bindable));
		}

		std::vector<Ref<Bindable>>::iterator begin()
		{
			return m_Bindables.begin();
		}

		std::vector<Ref<Bindable>>::iterator end()
		{
			return m_Bindables.end();
		}
	public:
		int m_PassID;
	private:
		std::vector<Ref<Bindable>> m_Bindables;
	};
}