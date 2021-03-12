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

	void Scene::OnUpdate(TimeStep ts, Ref<PointLight> light)
	{
		framebuffer->Bind();
		//Render
		auto& cameraGroup = m_Registry.view<TransformComponent, CameraComponent>();
		CameraComponent* cameraComponent = nullptr;
		TransformComponent* cameraTransform = nullptr;

		for (auto entity : cameraGroup)
		{
			auto&[ transform, camera] = cameraGroup.get<TransformComponent, CameraComponent>(entity);

			cameraComponent = &camera;
			cameraTransform = &transform;
		}

		if (cameraComponent == nullptr)
			return;

		DirectX::XMMATRIX cameraView = DirectX::XMMatrixInverse(nullptr,
			DirectX::XMMatrixRotationRollPitchYaw(cameraTransform->rotation.x, cameraTransform->rotation.y, cameraTransform->rotation.z) *
			DirectX::XMMatrixTranslation(cameraTransform->position.x, cameraTransform->position.y, cameraTransform->position.z)
		);

		Renderer::BeginScene();

		light->SetLightData(cameraView);

		auto renderGroup = m_Registry.group<ParentNodeComponent, MeshComponent>();
		for (auto entity : renderGroup)
		{
			auto& [node, mesh] = renderGroup.get<ParentNodeComponent, MeshComponent>(entity);
			TransformComponent& transform = m_Registry.get<TransformComponent>(entity);

			DirectX::XMMATRIX transformMat = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z) *
				DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z) *
											 node.initialTransform;

			for (int i = 0; i < mesh.m_NumMeshes; i++)
			{
				mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, cameraComponent->camera.GetProjection());
			}

			for (int i = 0; i < node.numChildren; i++)
			{
				//TODO: Change registry to group
				auto& [childTransform, childNode, childMesh] = m_Registry.get<TransformComponent, ChildNodeComponent, MeshComponent>(node.childNodes[i]);

				DrawChildren(childTransform, transformMat, childNode, childMesh, cameraView, cameraComponent->camera.GetProjection());
			}
		}
	}

	void Scene::DrawChildren(TransformComponent& transform, DirectX::FXMMATRIX& accumulatedTransform, ChildNodeComponent& node, MeshComponent& mesh, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
	{
		DirectX::XMMATRIX transformMat = DirectX::XMMatrixRotationRollPitchYaw(transform.rotation.x, transform.rotation.y, transform.rotation.z) *
										 DirectX::XMMatrixTranslation(transform.position.x, transform.position.y, transform.position.z) * 
										 node.initialTransform * 
										 accumulatedTransform;
		
		for (int i = 0; i < mesh.m_NumMeshes; i++)
		{
			mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, cameraProjection);
		}

		for (int i = 0; i < node.numChildren; i++)
		{
			//TODO: Change registry to group
			auto& [childTransform, childNode, childMesh] = m_Registry.get<TransformComponent, ChildNodeComponent, MeshComponent>(node.childNodes[i]);

			DrawChildren(childTransform, transformMat, childNode, childMesh, cameraView, cameraProjection);
		}
	}
}