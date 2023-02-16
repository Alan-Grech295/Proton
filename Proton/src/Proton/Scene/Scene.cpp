#include "ptpch.h"
#include "Scene.h"
#include "Entity.h"
#include "ScriptableEntity.h"
#include "Components.h"
#include "Proton\Core\Input.h"
#include "Proton\Core\KeyCodes.h"
#include "Proton\Scripting\ScriptEngine.h"

#include <cassert>

namespace Proton
{
	Scene::Scene()
	{
		/*FramebufferDescription desc;
		desc.Width = 1280;
		desc.Height = 720;
		desc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH };
		desc.ClearColor = new float[4]{ 0.02f, 0.07f, 0.2f, 1 };
	
		framebuffer = Framebuffer::Create(desc);*/

		//Creates camera entity
		//CreateEntity("Camera").AddComponent<CameraComponent>();

		//Editor Camera
		//m_EditorCam.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		//m_EditorCam.SetPerspective(45, 0.1f, 1000);

		//TEMP (debug lines)
		/*m_DebugPixShader = PixelShader::CreateUnique("C:\\Dev\\Proton\\Proton\\DebugPS.cso");
		m_DebugVertShader = VertexShader::CreateUnique("C:\\Dev\\Proton\\Proton\\DebugVS.cso");
		m_ViewProjBuffer = VertexConstantBuffer::CreateUnique(0, sizeof(DirectX::XMMATRIX), nullptr);

		BufferLayout layout = {
			{"Position", ShaderDataType::Float3},
			{"Color", ShaderDataType::Float3}
		};
		m_DebugVertBuffer = VertexBuffer::CreateUnique(layout, m_DebugVertShader.get());*/
	}

	Scene::~Scene()
	{

	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();

		for (auto it = view.rbegin(); it != view.rend(); ++it)
		{
			auto e = *it;
			UUID uuid = src.get<IDComponent>(e).ID;
			PT_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
			entt::entity dstEnttID = enttMap.at(uuid);

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		std::unordered_map<UUID, entt::entity> enttMap;
		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;
		
		//Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for(auto it = idView.rbegin(); it != idView.rend(); ++it)
		{
			auto e = *it;
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name, false);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		//Copy components (except IDComponent and TagComponent)
		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<MeshComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<RootNodeTag>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NodeComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<LightComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<ScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		//Setting scene view port size
		newScene->OnViewportResize(other->m_ViewportWidth, other->m_ViewportHeight);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		PT_CORE_ASSERT(m_EntityMap.find(uuid) != m_EntityMap.end(), "Entity not found");
		return m_EntityMap[uuid];
	}

	/*void Scene::DrawDebugLine(DirectX::XMFLOAT3 pointA, DirectX::XMFLOAT3 pointB, float r, float g, float b)
	{
		m_DebugVertBuffer->EmplaceBack(pointA, DirectX::XMFLOAT3{ r, g, b });
		m_DebugVertBuffer->EmplaceBack(pointB, DirectX::XMFLOAT3{ r, g, b });
	}*/

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, bool rootNode)
	{
		Entity entity = Entity(m_Registry.create(), this);
		entity.AddComponent<IDComponent>(uuid);
		m_EntityMap[uuid] = entity;

		entity.AddComponent<TransformComponent>(DirectX::XMFLOAT3{ 0, 0, 0 }, DirectX::XMFLOAT3{ 0, 0, 0 });
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		NodeComponent& nodeComponent = entity.AddComponent<NodeComponent>();

		if(rootNode)
			entity.AddComponent<RootNodeTag>();

		nodeComponent.ParentEntity = UUID::Null;
		nodeComponent.RootEntity = uuid;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_EntityMap.erase(entity.GetUUID());
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		//Scripting
		{
			ScriptEngine::OnRuntimeStart(this);
			//Instantiate all scripts
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = Entity(e, this);
				ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		ScriptEngine::OnRuntimeStop();
	}

	void Scene::OnRuntimeUpdate(TimeStep ts)
	{
		//Scripting
		{
			//C# Entity OnUpdate
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				ScriptEngine::OnUpdateEntity(entity, ts);
			}

			//Update Scripts
			PT_PROFILE_SCOPE("Script Updates");
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					//TODO: Move to OnScenePlay
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{ entity, this };

						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(ts);
			});
		}
	}

	void Scene::OnEditorUpdate(TimeStep ts)
	{
		
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		//m_EditorCam.SetViewportSize(width, height);

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);

			if (!cameraComponent.FixedAspectRatio)
			{
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);

		//Copy components (except IDComponent and TagComponent)
		CopyComponentIfExists<TransformComponent>(newEntity, entity);
		CopyComponentIfExists<CameraComponent>(newEntity, entity);
		CopyComponentIfExists<MeshComponent>(newEntity, entity);
		CopyComponentIfExists<RootNodeTag>(newEntity, entity);
		CopyComponentIfExists<NodeComponent>(newEntity, entity);
		CopyComponentIfExists<LightComponent>(newEntity, entity);
		CopyComponentIfExists<ScriptComponent>(newEntity, entity);
		CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);

		return newEntity;
	}

	/*void Scene::DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
	{
		auto [transform, node] = m_Registry.get<TransformComponent, NodeComponent>(entity);

		DirectX::XMMATRIX transformMat = transform.GetTransformMatrix() * 
										 node.m_Origin * 
										 accumulatedTransform;
		
		if (entity.HasComponent<MeshComponent>())
		{
			MeshComponent& mesh = entity.GetComponent<MeshComponent>();

			for (int i = 0; i < mesh.m_NumMeshes; i++)
			{
				mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, cameraProjection);
			}
		}

		for (int i = 0; i < node.m_ChildNodes.size(); i++)
		{
			DrawChildren(node.m_ChildNodes[i], transformMat, cameraView, cameraProjection);
		}
	}*/
}