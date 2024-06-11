#include "ptpch.h"
#include "SceneRenderer.h"
#include "Renderer.h"
#include "Proton\Scene\Entity.h"
#include "Proton/Model/Model.h"

namespace Proton
{
#define sceneRegistry m_Scene->m_Registry
	void SceneRenderer::Render(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix)
	{
		//RenderCommand::BindSwapChain();
		//RenderCommand::Clear();
		m_FrameBuffer->Clear();
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

		if (lightComponent)
		{
			DirectX::XMStoreFloat3(&(*lightCBuf)["pos"], DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&lightTransform->position), viewMatrix));
			(*lightCBuf)["ambient"] = lightComponent->Ambient;
			(*lightCBuf)["diffuseColor"] = lightComponent->DiffuseColour;
			(*lightCBuf)["diffuseIntensity"] = lightComponent->DiffuseIntensity;
			(*lightCBuf)["attConst"] = lightComponent->AttConst;
			(*lightCBuf)["attLin"] = lightComponent->AttLin;
			(*lightCBuf)["attQuad"] = lightComponent->AttQuad;
		}

		lightCBuf->Bind();

		//End Light Data

		//RenderCommand::SetTopology(TopologyType::TriangleList);

		auto renderView = sceneRegistry.view<RootNodeTag>();
		for (auto entity : renderView)
		{
			const auto& [node, transform] = sceneRegistry.get<NodeComponent, TransformComponent>(entity);
			SubmitNode(Entity(entity, m_Scene.get()), DirectX::XMMatrixIdentity(), viewMatrix, projMatrix);
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

		DirectX::XMMATRIX transformMat = transform.GetLocalTransformMatrix() *
										 node.Origin *
										 accumulatedTransform;

		if (entity.HasComponent<MeshRendererComponent>())
		{
			MeshRendererComponent& renderer = entity.GetComponent<MeshRendererComponent>();

			const auto modelView = transformMat * cameraView;
			const Transforms tf =
			{
				DirectX::XMMatrixTranspose(modelView),
				DirectX::XMMatrixTranspose(
					modelView *
					cameraProjection
				)
			};

			renderer.TransformBufferVert->SetData(&tf);
			(*renderer.TransformBufferPix)["modelViewProj"] = tf.modelViewProj;
			(*renderer.TransformBufferPix)["model"] = tf.model;

			SubmitMesh(entity, renderer.PMesh.get(), renderer.Materials, renderer.TransformBufferVert.get(), renderer.TransformBufferPix.get());
		}

		for (UUID e : node.Children)
		{
			SubmitNode(m_Scene->GetEntityByUUID(e), transformMat, cameraView, cameraProjection);
		}
	}

	void SceneRenderer::SubmitMesh(Entity entity, const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf)
	{
		Renderer::Submit(mesh, materials, vertTransformBuf, pixTransformBuf);
	}
}
