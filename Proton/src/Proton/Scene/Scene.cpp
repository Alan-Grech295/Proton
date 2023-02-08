#include "ptpch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include <cassert>
#include "Proton\Core\Input.h"
#include "Proton\Core\KeyCodes.h"

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
		CreateEntity("Camera").AddComponent<CameraComponent>();

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

	Entity Scene::CreateEntity(const std::string& name)
	{
		if (m_UUIDEntity[nextUUID])
			assert(false && ("Entity already exists!"));

		Entity entity = { m_Registry.create(), this };

		m_EntityUUID[entity] = nextUUID;
		m_UUIDEntity[nextUUID] = entity;

		nextUUID++;

		entity.AddComponent<TransformComponent>(DirectX::XMFLOAT3{ 0, 0, 0 }, DirectX::XMFLOAT3{ 0, 0, 0 });
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;

		NodeComponent& nodeComponent = entity.AddComponent<NodeComponent>();
		entity.AddComponent<RootNodeTag>();

		nodeComponent.m_ParentEntity = Entity::Null;
		nodeComponent.m_RootEntity = entity;

		return entity;
	}

	Entity Scene::GetEntityFromUUID(uint64_t uuid)
	{
		return Entity{ (entt::entity)m_UUIDEntity[uuid], this };
	}

	uint64_t Scene::GetUUIDFromEntity(Entity entity)
	{
		return m_EntityUUID[entity];
	}

	/*void Scene::DrawDebugLine(DirectX::XMFLOAT3 pointA, DirectX::XMFLOAT3 pointB, float r, float g, float b)
	{
		m_DebugVertBuffer->EmplaceBack(pointA, DirectX::XMFLOAT3{ r, g, b });
		m_DebugVertBuffer->EmplaceBack(pointB, DirectX::XMFLOAT3{ r, g, b });
	}*/

	Entity Scene::CreateEntityWithUUID(const uint64_t uuid, const std::string& name)
	{
		if (m_UUIDEntity[uuid])
			assert(false && ("Entity already exists!"));

		Entity entity = { m_Registry.create(), this };

		m_EntityUUID[entity] = uuid;
		m_UUIDEntity[uuid] = entity;

		if (nextUUID <= uuid)
			nextUUID = uuid + 1;

		entity.AddComponent<TransformComponent>(DirectX::XMFLOAT3{ 0, 0, 0 }, DirectX::XMFLOAT3{ 0, 0, 0 });
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;

		NodeComponent& nodeComponent = entity.AddComponent<NodeComponent>();
		entity.AddComponent<RootNodeTag>();

		nodeComponent.m_ParentEntity = Entity::Null;
		nodeComponent.m_RootEntity = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeUpdate(TimeStep ts)
	{
		//Update Scripts
		{
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
				cameraComponent.camera.SetViewportSize(width, height);
			}
		}
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