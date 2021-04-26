#include "ptpch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include <cassert>

namespace Proton
{
	Scene::Scene()
	{
		FramebufferDescription desc;
		desc.Width = 1280;
		desc.Height = 720;
	
		framebuffer = Framebuffer::Create(desc);
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

	void Scene::OnUpdate(TimeStep ts)
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

		framebuffer->Bind();
		//Render
		auto& cameraGroup = m_Registry.view<TransformComponent, CameraComponent>();
		CameraComponent* cameraComponent = nullptr;
		TransformComponent* cameraTransform = nullptr;

		for (auto entity : cameraGroup)
		{
			auto[ transform, camera] = cameraGroup.get<TransformComponent, CameraComponent>(entity);

			if (camera.Primary)
			{
				cameraComponent = &camera;
				cameraTransform = &transform;
				break;
			}
		}

		LightComponent* lightComponent = nullptr;
		TransformComponent* lightTransform = nullptr;
		auto lightView = m_Registry.view<LightComponent>();
		for (auto light : lightView)
		{
			lightComponent = &lightView.get<LightComponent>(light);
			lightTransform = &m_Registry.get<TransformComponent>(light);
		}

		if (cameraComponent == nullptr)
			return;

		DirectX::XMMATRIX cameraView = DirectX::XMMatrixInverse(nullptr,
									   DirectX::XMMatrixRotationRollPitchYaw(cameraTransform->rotation.x, cameraTransform->rotation.y, cameraTransform->rotation.z) *
									   DirectX::XMMatrixTranslation(cameraTransform->position.x, cameraTransform->position.y, cameraTransform->position.z)
		);

		Renderer::BeginScene();

		//light->SetLightData(cameraView);

		//Calculate Light Data (TEMP)
		PointLightData lightData = {};
		
		if (lightComponent != nullptr)
		{
			DirectX::XMStoreFloat3(&lightData.pos, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&lightTransform->position), cameraView));
			lightData.ambient = lightComponent->ambient;
			lightData.diffuseColor = lightComponent->diffuseColor;
			lightData.diffuseIntensity = lightComponent->diffuseIntensity;
			lightData.attConst = lightComponent->attConst;
			lightData.attLin = lightComponent->attLin;
			lightData.attQuad = lightComponent->attQuad;
			lightComponent->cbuf->SetData(sizeof(PointLightData), &lightData);
			lightComponent->cbuf->Bind();
		}

		//End Light Data

		auto renderView = m_Registry.view<RootNodeTag>();
		for (auto entity : renderView)
		{
			auto& [node, transform] = m_Registry.get<NodeComponent, TransformComponent>(entity);

			DirectX::XMMATRIX transformMat = transform.GetTransformMatrix() *
											 node.m_Origin;

			if (m_Registry.has<MeshComponent>(entity))
			{
				auto& mesh = m_Registry.get<MeshComponent>(entity);

				for (int i = 0; i < mesh.m_NumMeshes; i++)
				{
					mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, cameraComponent->camera.GetProjection());
				}
			}

			for (int i = 0; i < node.m_ChildNodes.size(); i++)
			{
				DrawChildren(node.m_ChildNodes[i], transformMat, cameraView, cameraComponent->camera.GetProjection());
			}
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

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

	void Scene::DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
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
	}
}