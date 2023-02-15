#pragma once
#include "Entity.h"

namespace Proton
{
	class ScriptableEntity
	{
		friend class Scene;
	public:
		virtual ~ScriptableEntity() {}
		template<typename Component>
		Component& GetComponent()
		{
			return m_Entity.GetComponent<Component>();
		}

	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(TimeStep ts) {}
	private:
		class Entity m_Entity;
	};
}