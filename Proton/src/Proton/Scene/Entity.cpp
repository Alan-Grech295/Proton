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

	void Entity::SetParent(UUID parentID, int pos)
	{
		std::string name = GetComponent<TagComponent>().Tag;
		UUID myUUID = GetUUID();

		if (parentID != UUID::Null)
		{
			NodeComponent& nodeComponent = GetComponent<NodeComponent>();
			if (nodeComponent.ParentEntity != UUID::Null && nodeComponent.ParentEntity == parentID && pos == -1)
				return;

			NodeComponent& parentNodeComponent = m_Scene->GetEntityByUUID(parentID).GetComponent<NodeComponent>();

			if (HasComponent<RootNodeTag>())
				RemoveComponent<RootNodeTag>();

			int childPos = INT32_MAX;

			if (nodeComponent.ParentEntity != UUID::Null)
			{
				NodeComponent& pastParentNodeComponent = m_Scene->GetEntityByUUID(nodeComponent.ParentEntity).GetComponent<NodeComponent>();
				for (int i = 0; i < pastParentNodeComponent.Children.size(); i++)
				{
					if (pastParentNodeComponent.Children[i] == myUUID)
					{
						childPos = i;
						pastParentNodeComponent.Children.erase(pastParentNodeComponent.Children.begin() + i);
						break;
					}
				}
			}

			if (pos > -1 && pos <= parentNodeComponent.Children.size())
			{
				int insertPos = pos;
				if (nodeComponent.ParentEntity == parentID && childPos < pos)
					insertPos--;

				parentNodeComponent.Children.insert(parentNodeComponent.Children.begin() + insertPos, myUUID);
			}
			else
			{
				parentNodeComponent.Children.push_back(myUUID);
			}

			nodeComponent.ParentEntity = parentID;

			if (m_Scene->GetEntityByUUID(parentID).HasComponent<RootNodeTag>())
				nodeComponent.RootEntity = parentID;
			else
				nodeComponent.RootEntity = parentNodeComponent.RootEntity;
		}
		else
		{
			if (!HasComponent<RootNodeTag>())
				AddComponent<RootNodeTag>();

			NodeComponent& nodeComponent = GetComponent<NodeComponent>();

			if (nodeComponent.ParentEntity != UUID::Null)
			{
				NodeComponent& parentNodeComponent = m_Scene->GetEntityByUUID(nodeComponent.ParentEntity).GetComponent<NodeComponent>();
				for (int i = 0; i < parentNodeComponent.Children.size(); i++)
				{
					if (parentNodeComponent.Children[i] == myUUID)
					{
						parentNodeComponent.Children.erase(parentNodeComponent.Children.begin() + i);
						break;
					}
				}
			}

			nodeComponent.ParentEntity = UUID::Null;
			nodeComponent.RootEntity = myUUID;
		}
	}

	const UUID Entity::GetUUID()
	{
		return GetComponent<IDComponent>().ID;
	}
	const std::string& Entity::GetName()
	{
		return GetComponent<TagComponent>().Tag;
	}

	template<typename T>
	void Entity::OnComponentAdded(T& component)
	{
		static_assert(sizeof(T) != 0, "Component cannot be of size 0");
	}

	template<>
	void Entity::OnComponentAdded<IDComponent>(IDComponent& component){}

	template<>
	void Entity::OnComponentAdded<TransformComponent>(TransformComponent& component) {}

	template<>
	void Entity::OnComponentAdded<TagComponent>(TagComponent& component) {}

	template<>
	void Entity::OnComponentAdded<CameraComponent>(CameraComponent& component) 
	{
		component.Camera.SetViewportSize(m_Scene->GetViewportWidth(), m_Scene->GetViewportHeight());
	}

	template<>
	void Entity::OnComponentAdded<StaticMeshComponent>(StaticMeshComponent& component) {}

	template<>
	void Entity::OnComponentAdded<RootNodeTag>(RootNodeTag& component) {}

	template<>
	void Entity::OnComponentAdded<NodeComponent>(NodeComponent& component) {}

	template<>
	void Entity::OnComponentAdded<LightComponent>(LightComponent& component) {}

	template<>
	void Entity::OnComponentAdded<ScriptComponent>(ScriptComponent& component) {}
	
	template<>
	void Entity::OnComponentAdded<NativeScriptComponent>(NativeScriptComponent& component){}
}