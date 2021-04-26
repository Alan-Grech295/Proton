#include "ptpch.h"
#include "Entity.h"
#include "Scene.h"
#include "Components.h"

namespace Proton
{
	Entity& Entity::Null = Entity{ entt::null, nullptr };
	Entity::Entity(entt::entity entityHandler, Scene* scene)
		:
		m_EntityHandle(entityHandler),
		m_Scene(scene)
	{
	}

	void Entity::SetParent(Entity* parent, int pos)
	{
		if (parent != nullptr)
		{
			NodeComponent& nodeComponent = GetComponent<NodeComponent>();
			if (nodeComponent.m_ParentEntity != Entity::Null && nodeComponent.m_ParentEntity == *parent && pos == -1)
				return;

			NodeComponent& parentNodeComponent = parent->GetComponent<NodeComponent>();

			if (HasComponent<RootNodeTag>())
				RemoveComponent<RootNodeTag>();

			int childPos = INT32_MAX;

			if (nodeComponent.m_ParentEntity != Entity::Null)
			{
				NodeComponent& pastParentNodeComponent = nodeComponent.m_ParentEntity.GetComponent<NodeComponent>();
				for (int i = 0; i < pastParentNodeComponent.m_ChildNodes.size(); i++)
				{
					if (pastParentNodeComponent.m_ChildNodes[i] == *this)
					{
						childPos = i;
						pastParentNodeComponent.m_ChildNodes.erase(pastParentNodeComponent.m_ChildNodes.begin() + i);
						break;
					}
				}
			}

			if (pos > -1 && pos < parentNodeComponent.m_ChildNodes.size() + 1)
			{
				int insertPos = pos;
				if (nodeComponent.m_ParentEntity == *parent && childPos < pos)
					insertPos -= 1;

				parentNodeComponent.m_ChildNodes.insert(parentNodeComponent.m_ChildNodes.begin() + insertPos, *this);
			}
			else
			{
				parentNodeComponent.m_ChildNodes.push_back(*this);
			}

			nodeComponent.m_ParentEntity = *parent;

			if (parent->HasComponent<RootNodeTag>())
				nodeComponent.m_RootEntity = *parent;
			else
				nodeComponent.m_RootEntity = parentNodeComponent.m_RootEntity;
		}
		else
		{
			if (!HasComponent<RootNodeTag>())
				AddComponent<RootNodeTag>();

			NodeComponent& nodeComponent = GetComponent<NodeComponent>();

			if (nodeComponent.m_ParentEntity != Entity::Null)
			{
				NodeComponent& parentNodeComponent = nodeComponent.m_ParentEntity.GetComponent<NodeComponent>();
				for (int i = 0; i < parentNodeComponent.m_ChildNodes.size(); i++)
				{
					if (parentNodeComponent.m_ChildNodes[i] == *this)
					{
						parentNodeComponent.m_ChildNodes.erase(parentNodeComponent.m_ChildNodes.begin() + i);
						break;
					}
				}
			}

			nodeComponent.m_ParentEntity = Entity::Null;
			nodeComponent.m_RootEntity = *this;
		}
	}
}