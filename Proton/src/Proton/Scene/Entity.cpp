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
		std::string name = GetComponent<TagComponent>().tag;
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
				for (int i = 0; i < pastParentNodeComponent.m_Children.size(); i++)
				{
					if (pastParentNodeComponent.m_Children[i] == *this)
					{
						childPos = i;
						pastParentNodeComponent.m_Children.erase(pastParentNodeComponent.m_Children.begin() + i);
						break;
					}
				}
			}

			if (pos > -1 && pos < parentNodeComponent.m_Children.size() + 1)
			{
				int insertPos = pos;
				if (nodeComponent.m_ParentEntity == *parent && childPos < pos)
					insertPos -= 1;

				parentNodeComponent.m_Children.insert(parentNodeComponent.m_Children.begin() + insertPos, *this);
			}
			else
			{
				parentNodeComponent.m_Children.push_back(*this);
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
				for (int i = 0; i < parentNodeComponent.m_Children.size(); i++)
				{
					if (parentNodeComponent.m_Children[i] == *this)
					{
						parentNodeComponent.m_Children.erase(parentNodeComponent.m_Children.begin() + i);
						break;
					}
				}
			}

			nodeComponent.m_ParentEntity = Entity::Null;
			nodeComponent.m_RootEntity = *this;
		}
	}
}