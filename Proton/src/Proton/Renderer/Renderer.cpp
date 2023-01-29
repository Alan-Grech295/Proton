#include "ptpch.h"
#include "Renderer.h"
#include "Proton\Model\Model.h"
#include "Proton\Model\Model.h"

namespace Proton
{
	std::array<Pass, 1> Renderer::m_RenderQueue = {Pass("Lambertian")};

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

	void Renderer::Submit(Mesh& mesh)
	{
		PT_PROFILE_FUNCTION();

		for (Technique& t : mesh.m_Techniques)
		{
			for (Step& s : t)
			{
				assert("Invalid pass ID" && (s.m_PassID < 0 && s.m_PassID >= m_RenderQueue.size()));
				m_RenderQueue[s.m_PassID].AddJob({ &mesh, &s });
			}
		}
	}

	void Renderer::Submit(Mesh* mesh)
	{
		PT_PROFILE_FUNCTION();

		for (Technique& t : mesh->m_Techniques)
		{
			for (Step& s : t)
			{
				assert("Invalid pass ID" && !(s.m_PassID < 0 || s.m_PassID >= m_RenderQueue.size()));
				m_RenderQueue[s.m_PassID].AddJob({ mesh, &s });
			}
		}
	}

	void Renderer::Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer)
	{
		vertBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(indexBuffer->size());
	}

	void Renderer::Render()
	{
		for (Pass& p : m_RenderQueue)
		{
			for (Job& job : p)
			{
				//Bind all bindables
				job.m_Mesh->m_IndexBuffer->Bind();
				job.m_Mesh->m_VertBuffer->Bind();
				job.m_Mesh->m_Topology->Bind();

				//
				job.m_Mesh->m_TransformCBuf->Bind();
				job.m_Mesh->m_TransformCBufPix->Bind();

				for (Ref<Bindable> bind : *job.m_Step)
				{
					bind->Bind();
				}

				RenderCommand::DrawIndexed(job.m_Mesh->m_IndexBuffer->size());
			}
		}
	}

	void Renderer::DrawCall(const UINT count)
	{
		PT_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(count);
	}
}