#pragma once
#include "Scene.h"
#include "entt.hpp"

namespace Proton
{
	struct NativeScriptComponent;
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity entityHandler, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename Component, typename... Args>
		Component& AddComponent(Args&&... args)
		{
			return m_Scene->m_Registry.emplace<Component>(m_EntityHandle, std::forward<Args>(args)...);
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

		operator bool() const { return (std::uint32_t)m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return (uint32_t)m_EntityHandle; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }
		bool operator!=(const Entity& other) const { return !(*this == other); }
	private:
		entt::entity m_EntityHandle{ entt::null };
		Scene* m_Scene = nullptr;
	};
}