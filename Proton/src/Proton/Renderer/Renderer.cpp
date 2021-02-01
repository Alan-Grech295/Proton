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

	void Renderer::Submit(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer)
	{
		vertBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(vertBuffer, indexBuffer);
	}
}