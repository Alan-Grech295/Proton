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
		FramebufferDescription desc;
		desc.Width = 1280;
		desc.Height = 720;
		desc.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH };
		desc.ClearColor = new float[4]{ 0.02f, 0.07f, 0.2f, 1 };
	
		framebuffer = Framebuffer::Create(desc);

		//Creates camera entity
		CreateEntity("Camera").AddComponent<CameraComponent>();

		//Editor Camera
		m_EditorCam.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		m_EditorCam.SetPerspective(45, 0.1f, 1000);

		//TEMP (debug lines)
		m_DebugPixShader = PixelShader::CreateUnique("C:\\Dev\\Proton\\Proton\\DebugPS.cso");
		m_DebugVertShader = VertexShader::CreateUnique("C:\\Dev\\Proton\\Proton\\DebugVS.cso");
		m_ViewProjBuffer = VertexConstantBuffer::CreateUnique(0, sizeof(DirectX::XMMATRIX), nullptr);

		BufferLayout layout = {
			{"Position", ShaderDataType::Float3},
			{"Color", ShaderDataType::Float3}
		};
		m_DebugVertBuffer = VertexBuffer::CreateUnique(layout, m_DebugVertShader.get());
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

	void Scene::DrawDebugLine(DirectX::XMFLOAT3 pointA, DirectX::XMFLOAT3 pointB, float r, float g, float b)
	{
		m_DebugVertBuffer->EmplaceBack(pointA, DirectX::XMFLOAT3{ r, g, b });
		m_DebugVertBuffer->EmplaceBack(pointB, DirectX::XMFLOAT3{ r, g, b });
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

		framebuffer->Bind();
		//Render
		auto& cameraGroup = m_Registry.view<TransformComponent, CameraComponent>();
		CameraComponent* cameraComponent = nullptr;
		TransformComponent* cameraTransform = nullptr;

		for (auto entity : cameraGroup)
		{
			auto[transform, camera] = cameraGroup.get<TransformComponent, CameraComponent>(entity);

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

		//Calculate Light Data (TEMP)
		PointLightData lightData = {};
		
		if (lightComponent)
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

		RenderCommand::SetTopology(RendererAPI::Topology::Triangle);

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

	void Scene::OnEditorUpdate(TimeStep ts)
	{
		if (m_UpdateEditorCam)
		{
			float speed = ts * m_EditorCamSpeed;

			DirectX::XMFLOAT3 localMove = { 0, 0, 0 };

			if (Input::IsKeyPressed(Key::D))
				localMove.x += speed;

			if (Input::IsKeyPressed(Key::A))
				localMove.x -= speed;

			if (Input::IsKeyPressed(Key::E))
				localMove.y += speed;

			if (Input::IsKeyPressed(Key::Q))
				localMove.y -= speed;

			if (Input::IsKeyPressed(Key::W))
				localMove.z += speed;

			if (Input::IsKeyPressed(Key::S))
				localMove.z -= speed;

			m_EditorCamRotation.x += m_EditorCamRotationSpeed * Input::GetMouseDeltaY() * std::max<float>(0.001f, ts);
			m_EditorCamRotation.y += m_EditorCamRotationSpeed * Input::GetMouseDeltaX() * std::max<float>(0.001f, ts);

			DirectX::XMStoreFloat3(&localMove, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&localMove), DirectX::XMMatrixRotationRollPitchYaw(m_EditorCamRotation.x, m_EditorCamRotation.y, m_EditorCamRotation.z)));

			m_EditorCamPosition.x += localMove.x;
			m_EditorCamPosition.y += localMove.y;
			m_EditorCamPosition.z += localMove.z;
		}
		
		framebuffer->Bind();
		//Render

		LightComponent* lightComponent = nullptr;
		TransformComponent* lightTransform = nullptr;
		auto lightView = m_Registry.view<LightComponent>();
		for (auto light : lightView)
		{
			lightComponent = &lightView.get<LightComponent>(light);
			lightTransform = &m_Registry.get<TransformComponent>(light);
		}

		DirectX::XMMATRIX cameraView = DirectX::XMMatrixInverse(nullptr,
									   DirectX::XMMatrixRotationRollPitchYaw(m_EditorCamRotation.x, m_EditorCamRotation.y, m_EditorCamRotation.z) *
									   DirectX::XMMatrixTranslation(m_EditorCamPosition.x, m_EditorCamPosition.y, m_EditorCamPosition.z)
		);



		Renderer::BeginScene();

		//Calculate Light Data (TEMP)
		PointLightData lightData = {};

		if (lightComponent)
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

		RenderCommand::SetTopology(RendererAPI::Topology::Triangle);

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
					mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, m_EditorCam.GetProjection());
				}
			}

			for (int i = 0; i < node.m_ChildNodes.size(); i++)
			{
				DrawChildren(node.m_ChildNodes[i], transformMat, cameraView, m_EditorCam.GetProjection());
			}
		}

		//TEMP Render Debug Lines
		m_DebugVertBuffer->Bind();
		m_DebugVertShader->Bind();
		m_DebugPixShader->Bind();

		DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(cameraView * m_EditorCam.GetProjection());

		m_ViewProjBuffer->SetData(sizeof(viewProj), &viewProj);
		m_ViewProjBuffer->Bind();

		RenderCommand::SetTopology(RendererAPI::Topology::Line);
		RenderCommand::Draw(m_DebugVertBuffer->size());
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportWidth = width;
		m_ViewportHeight = height;

		m_EditorCam.SetViewportSize(width, height);

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