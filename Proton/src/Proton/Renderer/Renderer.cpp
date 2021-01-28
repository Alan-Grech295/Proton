#include "ptpch.h"
#include "Renderer.h"

namespace Proton
{
	void Renderer::BeginScene()
	{

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