#include "ptpch.h"
#include "Renderer.h"


namespace Proton
{
	Camera* Renderer::m_Camera = nullptr;

	void Renderer::BeginScene(Camera& camera)
	{
		m_Camera = &camera;
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(Model& model)
	{
		model.Bind(std::bind(&Renderer::DrawCall, std::placeholders::_1), DirectX::XMMatrixIdentity());
	}

	void Renderer::Submit(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer)
	{
		vertBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(indexBuffer->GetCount());
	}

	void Renderer::DrawCall(const UINT count)
	{
		RenderCommand::DrawIndexed(count);
	}
}