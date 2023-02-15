#include "ptpch.h"
#include "Entity.h"
#include "Scene.h"
//#include "Components.h"

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
		std::string name = GetComponent<TagComponent>().Tag;
		if (parent != nullptr)
		{
			NodeComponent& nodeComponent = GetComponent<NodeComponent>();
			if (nodeComponent.ParentEntity != Entity::Null && nodeComponent.ParentEntity == *parent && pos == -1)
				return;

			NodeComponent& parentNodeComponent = parent->GetComponent<NodeComponent>();

			if (HasComponent<RootNodeTag>())
				RemoveComponent<RootNodeTag>();

			int childPos = INT32_MAX;

			if (nodeComponent.ParentEntity != Entity::Null)
			{
				NodeComponent& pastParentNodeComponent = nodeComponent.ParentEntity.GetComponent<NodeComponent>();
				for (int i = 0; i < pastParentNodeComponent.Children.size(); i++)
				{
					if (pastParentNodeComponent.Children[i] == *this)
					{
						childPos = i;
						pastParentNodeComponent.Children.erase(pastParentNodeComponent.Children.begin() + i);
						break;
					}
				}
			}

			if (pos > -1 && pos < parentNodeComponent.Children.size() + 1)
			{
				int insertPos = pos;
				if (nodeComponent.ParentEntity == *parent && childPos < pos)
					insertPos -= 1;

				parentNodeComponent.Children.insert(parentNodeComponent.Children.begin() + insertPos, *this);
			}
			else
			{
				parentNodeComponent.Children.push_back(*this);
			}

			nodeComponent.ParentEntity = *parent;

			if (parent->HasComponent<RootNodeTag>())
				nodeComponent.RootEntity = *parent;
			else
				nodeComponent.RootEntity = parentNodeComponent.RootEntity;
		}
		else
		{
			if (!HasComponent<RootNodeTag>())
				AddComponent<RootNodeTag>();

			NodeComponent& nodeComponent = GetComponent<NodeComponent>();

			if (nodeComponent.ParentEntity != Entity::Null)
			{
				NodeComponent& parentNodeComponent = nodeComponent.ParentEntity.GetComponent<NodeComponent>();
				for (int i = 0; i < parentNodeComponent.Children.size(); i++)
				{
					if (parentNodeComponent.Children[i] == *this)
					{
						parentNodeComponent.Children.erase(parentNodeComponent.Children.begin() + i);
						break;
					}
				}
			}

			nodeComponent.ParentEntity = Entity::Null;
			nodeComponent.RootEntity = *this;
		}
	}

	const UUID Entity::GetUUID()
	{
		return GetComponent<IDComponent>().ID;
	}
}