#include "ptpch.h"
#include "SceneRenderer.h"
#include "Proton\Scene\Components.h"

namespace Proton
{
#define sceneRegistry m_Scene->m_Registry
	void SceneRenderer::Render(const Camera& camera, DirectX::FXMMATRIX& cameraView)
	{
		m_FrameBuffer->Bind();

		//TODO: Replace with better light code
		LightComponent* lightComponent = nullptr;
		TransformComponent* lightTransform = nullptr;
		auto lightView = sceneRegistry.view<LightComponent>();
		for (auto light : lightView)
		{
			lightComponent = &lightView.get<LightComponent>(light);
			lightTransform = &sceneRegistry.get<TransformComponent>(light);
		}

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

		auto renderView = sceneRegistry.view<RootNodeTag>();
		for (auto entity : renderView)
		{
			auto& [node, transform] = sceneRegistry.get<NodeComponent, TransformComponent>(entity);

			DirectX::XMMATRIX transformMat = transform.GetTransformMatrix() *
				node.m_Origin;

			if (sceneRegistry.has<MeshComponent>(entity))
			{
				auto& mesh = m_Scene->m_Registry.get<MeshComponent>(entity);

				for (int i = 0; i < mesh.m_NumMeshes; i++)
				{
					mesh.m_MeshPtrs[i]->Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), transformMat, cameraView, camera.GetProjection());
				}
			}

			for (int i = 0; i < node.m_ChildNodes.size(); i++)
			{
				DrawChildren(node.m_ChildNodes[i], transformMat, cameraView, camera.GetProjection());
			}
		}

		//TEMP Render Debug Lines
		m_Scene->m_DebugVertBuffer->Bind();
		m_Scene->m_DebugVertShader->Bind();
		m_Scene->m_DebugPixShader->Bind();

		DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(cameraView * camera.GetProjection());

		m_Scene->m_ViewProjBuffer->SetData(sizeof(viewProj), &viewProj);
		m_Scene->m_ViewProjBuffer->Bind();

		RenderCommand::SetTopology(RendererAPI::Topology::Line);
		RenderCommand::Draw(m_Scene->m_DebugVertBuffer->size());
	}
	void SceneRenderer::DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
	{
		auto [transform, node] = sceneRegistry.get<TransformComponent, NodeComponent>(entity);

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
