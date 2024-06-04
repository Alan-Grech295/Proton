#include "ptpch.h"
#include "Renderer.h"
#include "Proton\Model\Model.h"
#include "Proton/Core/Core.h"
#include "Proton\Renderer\Render Queue\Pass.h"

namespace Proton
{
	std::array<Pass, 1> Renderer::m_RenderQueue = {Pass("Opaque")};

	void Renderer::BeginScene()
	{
		for (Pass& pass : m_RenderQueue)
		{
			pass.Clear();
		}
	}

	void Renderer::EndScene()
	{

	}

	void Renderer::Submit(const Mesh* mesh, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf)
	{
		PT_PROFILE_FUNCTION();

		for (Ref<Material::Pass> t : mesh->material->m_Passes)
		{
			PT_CORE_ASSERT(t->m_PassID >= 0 && t->m_PassID < m_RenderQueue.size(), "Invalid pass ID");
			m_RenderQueue[t->m_PassID].AddJob({ mesh, vertTransformBuf, pixTransformBuf, t });
		}
	}

	void Renderer::Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer)
	{
		vertBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(indexBuffer->size());
	}

	int Renderer::GetPassIDFromName(const std::string& name)
	{
		for (int i = 0; i < m_RenderQueue.size(); i++)
		{
			if (m_RenderQueue[i].m_Name == name)
				return i;
		}

		PT_CORE_ASSERT(false, "Pass name not found!");
		return -1;
	}

	void Renderer::Render()
	{
		for (Pass& p : m_RenderQueue)
		{
			for (Job& job : p)
			{
				// Binding the shaders
				job.MaterialPass->m_VertexShader->Bind();
				job.MaterialPass->m_PixelShader->Bind();

				// Bind all bindables
				job.Mesh->m_IndexBuffer->Bind();
				job.Mesh->m_VertexBuffer->Bind();
				job.Mesh->m_Topology->Bind();

				// Binding transform buffers
				job.VertConstBuf->Bind();
				job.PixConstBuf->Bind();

				for (Ref<Bindable> bind : job.MaterialPass->m_Bindables)
					bind->Bind();

				RenderCommand::DrawIndexed(job.Mesh->m_IndexBuffer->size());
			}
		}
	}

	void Renderer::DrawCall(const UINT count)
	{
		PT_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(count);
	}
}