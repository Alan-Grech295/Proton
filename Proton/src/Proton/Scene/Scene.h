#pragma once
#include "Proton\Core\TimeStep.h"
#include "Proton\Renderer\Framebuffer.h"
#include "Proton\Renderer\Bindables\Buffer.h"
#include "SceneCamera.h"
#include "Proton\Renderer\EditorCamera.h"
#include "Proton\Core\UUID.h"

#include "entt.hpp"
#include <unordered_map>

namespace Proton
{
	class Entity;
	struct TransformComponent;
	struct NodeComponent;
	struct StaticMeshComponent;

	class Scene
	{
		//TODO: Remove friend class?
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class EditorLayer;

		friend class SceneRenderer;
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = "");
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "", bool rootNode = true);
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnRuntimeUpdate(TimeStep ts);
		void OnEditorUpdate(TimeStep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
		uint32_t GetViewportWidth() { return m_ViewportWidth; }
		uint32_t GetViewportHeight() { return m_ViewportHeight; }

		Entity DuplicateEntity(Entity entity);

		Entity GetEntityByUUID(UUID uuid);

		Entity FindEntityByName(std::string_view name);

		template<typename T>
		inline Entity FindEntityWithComponent()
		{
			const auto& view = m_Registry.view<T>();

			if (view.empty())
				return Entity::Null;
			else
				return Entity{ view[0], this };
		}

		void ClearEntities()
		{
			m_Registry = entt::registry();
		}

		bool IsRunning() { return m_IsRunning; }
		bool IsPaused() { return m_IsPaused; }

		void SetPaused(bool paused) { m_IsPaused = paused; }

		void Step(int frames = 1);

		//TEMP
		//void DrawDebugLine(DirectX::XMFLOAT3 pointA, DirectX::XMFLOAT3 pointB, float r, float g, float b);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		bool m_IsRunning = false;
		bool m_IsPaused = false;
		int m_StepFrames = 0;

		std::unordered_map<UUID, Entity> m_EntityMap;

		//TEMP (for debug lines)
		/*Scope<VertexBuffer> m_DebugVertBuffer;
		Scope<VertexShader> m_DebugVertShader;
		Scope<PixelShader> m_DebugPixShader;
		Scope<VertexConstantBuffer> m_ViewProjBuffer;*/
	};
}