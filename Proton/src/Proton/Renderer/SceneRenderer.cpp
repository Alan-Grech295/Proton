#include "ptpch.h"
#include "SceneRenderer.h"
#include "Proton\Scene\Components.h"

namespace Proton
{
#define sceneRegistry m_Scene->m_Registry
	void SceneRenderer::Render(const EditorCamera& editorCam)
	{
		m_FrameBuffer->Bind();
		Renderer::BeginScene();

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
			DirectX::XMStoreFloat3(&lightData.pos, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&lightTransform->position), editorCam.GetViewMatrix()));
			lightData.ambient = lightComponent->ambient;
			lightData.diffuseColor = lightComponent->diffuseColour;
			lightData.diffuseIntensity = lightComponent->diffuseIntensity;
			lightData.attConst = lightComponent->attConst;
			lightData.attLin = lightComponent->attLin;
			lightData.attQuad = lightComponent->attQuad;
		}

		lightCBuf->SetData(&lightData);
		lightCBuf->Bind();

		//End Light Data

		//RenderCommand::SetTopology(TopologyType::TriangleList);

		auto renderView = sceneRegistry.view<RootNodeTag>();
		for (auto entity : renderView)
		{
			auto& [node, transform] = sceneRegistry.get<NodeComponent, TransformComponent>(entity);

			SubmitNode(Entity(entity, m_Scene.get()), DirectX::XMMatrixIdentity(), editorCam.GetViewMatrix(), editorCam.GetProjection());
		}

		Renderer::Render();
		
		//TEMP Render Debug Lines
		/*m_Scene->m_DebugVertBuffer->Bind();
		m_Scene->m_DebugVertShader->Bind();
		m_Scene->m_DebugPixShader->Bind();

		DirectX::XMMATRIX viewProj = DirectX::XMMatrixTranspose(editorCam.GetViewProjection());

		m_Scene->m_ViewProjBuffer->SetData(sizeof(viewProj), &viewProj);
		m_Scene->m_ViewProjBuffer->Bind();

		RenderCommand::SetTopology(RendererAPI::Topology::Line);
		RenderCommand::Draw(m_Scene->m_DebugVertBuffer->size());*/
	}

	void SceneRenderer::SubmitNode(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
	{
		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};

		auto [transform, node] = sceneRegistry.get<TransformComponent, NodeComponent>(entity);

		DirectX::XMMATRIX transformMat = transform.GetTransformMatrix() *
										 node.m_Origin *
										 accumulatedTransform;

		if (entity.HasComponent<MeshComponent>())
		{
			MeshComponent& mesh = entity.GetComponent<MeshComponent>();

			for (Mesh* m : mesh.m_MeshPtrs)
			{
				const auto modelView = transformMat * cameraView;
				const Transforms tf =
				{
					DirectX::XMMatrixTranspose(modelView),
					DirectX::XMMatrixTranspose(
						modelView *
						cameraProjection
					)
				};

				m->m_TransformCBuf->SetData(&tf);
				m->m_TransformCBufPix->SetData(&tf);

				Renderer::Submit(m);
			}
		}

		for (Entity& e : node.m_Children)
		{
			SubmitNode(e, transformMat, cameraView, cameraProjection);
		}
	}
	
	/*void SceneRenderer::DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection)
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
	}*/
}
