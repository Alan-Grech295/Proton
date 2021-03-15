#include "ptpch.h"
#include "Scene.h"
#include "Components.h"
#include "Entity.h"

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
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<TransformComponent>(DirectX::XMFLOAT3{ 0, 0, 0 }, DirectX::XMFLOAT3{ 0, 0, 0 });
		auto& tag = entity.AddComponent<TagComponent>();
		tag.tag = name.empty() ? "Entity" : name;

		return entity;
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

		auto renderGroup = m_Registry.group<ParentNodeComponent, MeshComponent>();
		for (auto entity : renderGroup)
		{
			auto [node, mesh] = renderGroup.get<ParentNodeComponent, MeshComponent>(entity);
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);

			DirectX::XMMATRIX transformMat = transform.GetTransformMatrix() *
											 node.initialTransform;

			for (int i = 0; i < mesh.m_NumMeshes; i++)
			{
				mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, cameraComponent->camera.GetProjection());
			}

			for (int i = 0; i < node.numChildren; i++)
			{
				//TODO: Change registry to group
				auto [childTransform, childNode, childMesh] = m_Registry.get<TransformComponent, ChildNodeComponent, MeshComponent>(node.childNodes[i]);

				DrawChildren(childTransform, transformMat, childNode, childMesh, cameraView, cameraComponent->camera.GetProjection());
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

	void Scene::DrawChildren(TransformComponent& transform, DirectX::FXMMATRIX& accumulatedTransform, ChildNodeComponent& node, MeshComponent& mesh, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
	{
		DirectX::XMMATRIX transformMat = transform.GetTransformMatrix() * 
										 node.initialTransform * 
										 accumulatedTransform;
		
		for (int i = 0; i < mesh.m_NumMeshes; i++)
		{
			mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, cameraProjection);
		}

		for (int i = 0; i < node.numChildren; i++)
		{
			//TODO: Change registry to group
			auto [childTransform, childNode, childMesh] = m_Registry.get<TransformComponent, ChildNodeComponent, MeshComponent>(node.childNodes[i]);
			
			DrawChildren(childTransform, transformMat, childNode, childMesh, cameraView, cameraProjection);
		}
	}
}