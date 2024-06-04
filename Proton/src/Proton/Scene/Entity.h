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
			Component& component = m_Scene->m_Registry.emplace<Component>(m_EntityHandle, std::forward<Args>(args)...);
			OnComponentAdded(component);
			return component;
		}

		template<typename Component, typename... Args>
		Component& AddOrReplaceComponent(Args&&... args)
		{
			Component& component = m_Scene->m_Registry.emplace_or_replace<Component>(m_EntityHandle, std::forward<Args>(args)...);
			OnComponentAdded(component);
			return component;
		}

		template<typename Component>
		Component& GetComponent()
		{
			return m_Scene->m_Registry.get<Component>(m_EntityHandle);
		}

		template<typename Component>
		bool HasComponent()
		{
			return m_Scene->m_Registry.any_of<Component>(m_EntityHandle);
		}

		template<typename Component>
		void RemoveComponent()
		{
			m_Scene->m_Registry.remove<Component>(m_EntityHandle);
		}

		template<typename T>
		void AddScript();

		void SetParent(UUID parentID = UUID::Null, int pos = -1);

		virtual const UUID GetUUID();
		virtual const std::string& GetName();

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator UUID() { return GetUUID(); }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }

	private:
		template<typename T>
		void OnComponentAdded(T& component);
	public:
		static Entity Null;
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
