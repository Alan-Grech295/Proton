#pragma once
#include "entt.hpp"
#include "Proton\Core\TimeStep.h"
#include "Proton\Renderer\Framebuffer.h"
#include "Proton\Renderer\Bindables\Buffer.h"
#include "SceneCamera.h"
#include "Proton\Renderer\EditorCamera.h"
#include <unordered_map>

namespace Proton
{
	class Entity;
	struct TransformComponent;
	struct NodeComponent;
	struct MeshComponent;

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

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeUpdate(TimeStep ts);
		void OnEditorUpdate(TimeStep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);
		uint32_t GetViewportWidth() { return m_ViewportWidth; }
		uint32_t GetViewportHeight() { return m_ViewportHeight; }

		Entity GetEntityFromUUID(uint64_t uuid);
		uint64_t GetUUIDFromEntity(Entity entity);

		Entity FindEntityWithTag(const std::string& tag);

		//void SetUpdateEditorCamera(bool update) { m_UpdateEditorCam = update; }

		template<typename T>
		inline Entity FindEntityWithComponent()
		{
			auto& view = m_Registry.view<T>();

			if (view.empty())
				return Entity::Null;
			else
				return Entity{ view[0], this };
		}

		void ClearEntities()
		{
			m_Registry = entt::registry();
		}

		//TEMP
		//void DrawDebugLine(DirectX::XMFLOAT3 pointA, DirectX::XMFLOAT3 pointB, float r, float g, float b);
	private:
		Entity CreateEntityWithUUID(const uint64_t uuid = 0, const std::string& name = std::string());
		//void DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection);
	public:
		//TEMP
		/*struct PointLightData
		{
			alignas(16) DirectX::XMFLOAT3 pos;
			alignas(16) DirectX::XMFLOAT3 ambient;
			alignas(16) DirectX::XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		};*/
	private:
		//Ref<Framebuffer> framebuffer;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		uint64_t nextUUID = 0;

		std::unordered_map<uint32_t, uint64_t> m_EntityUUID;
		std::unordered_map<uint64_t, uint32_t> m_UUIDEntity;

		//TEMP (for debug lines)
		/*Scope<VertexBuffer> m_DebugVertBuffer;
		Scope<VertexShader> m_DebugVertShader;
		Scope<PixelShader> m_DebugPixShader;
		Scope<VertexConstantBuffer> m_ViewProjBuffer;*/
	};
}