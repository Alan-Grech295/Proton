#pragma once
#include "entt.hpp"
#include "Scene.h"
#include "Proton/Core/UUID.h"

namespace Proton
{
	//Forward declaration
	struct NativeScriptComponent;

	class Entity
	{
		friend class EntityHash;
	public:
		Entity() = default;
		Entity(entt::entity entityHandler, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename Component, typename... Args>
		Component& AddComponent(Args&&... args)
		{
			return m_Scene->m_Registry.emplace<Component>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename Component, typename... Args>
		Component& AddOrReplaceComponent(Args&&... args)
		{
			return m_Scene->m_Registry.emplace_or_replace<Component>(m_EntityHandle, std::forward<Args>(args)...);
		}

		template<typename Component>
		Component& GetComponent()
		{
			return m_Scene->m_Registry.get<Component>(m_EntityHandle);
		}

		template<typename Component>
		bool HasComponent()
		{
			return m_Scene->m_Registry.has<Component>(m_EntityHandle);
		}

		template<typename Component>
		void RemoveComponent()
		{
			m_Scene->m_Registry.remove<Component>(m_EntityHandle);
		}

		template<typename T>
		void AddScript()
		{
			AddComponent<NativeScriptComponent>().Bind<T>();
		}

		void SetParent(UUID parentID = UUID::Null, int pos = -1);

		const UUID GetUUID();
		const std::string& GetName();

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
	public:
		static Entity& Null;
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};

	class EntityHash {
	public:

		// Use sum of lengths of first and last names 
		// as hash function. 
		size_t operator()(const Entity& e) const
		{
			return (size_t)(uint32_t)e + (size_t)e.m_Scene;
		}
	};
}

#include "Components.h"
