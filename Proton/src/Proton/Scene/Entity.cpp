#include "ptpch.h"
#include "Entity.h"

namespace Proton
{
	Entity::Entity(entt::entity entityHandler, Scene* scene)
		:
		m_EntityHandle(entityHandler),
		m_Scene(scene)
	{

	}
}