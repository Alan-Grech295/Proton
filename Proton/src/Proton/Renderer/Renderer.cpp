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

	void Renderer::Submit(Model& model)
	{
		PT_PROFILE_FUNCTION();
	}

	void Renderer::Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer)
	{
		vertBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(indexBuffer->size());
	}

	void Renderer::DrawCall(const UINT count)
	{
		PT_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(count);
	}
}