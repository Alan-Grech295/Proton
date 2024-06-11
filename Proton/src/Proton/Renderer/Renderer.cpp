#include "ptpch.h"
#include "Renderer.h"
#include "Proton\Model\Model.h"
#include "Proton/Core/Core.h"
#include "Proton\Renderer\Render Queue\Pass.h"

namespace Proton
{
	std::array<Pass, 1> Renderer::m_RenderQueue = {Pass("Opaque")};
	std::array<std::vector<std::function<void()>>, 1> Renderer::m_PreRenderCallbacks = { 
		std::vector<std::function<void()>>(), 
	};

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

	void Renderer::Submit(const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf)
	{
		PT_PROFILE_FUNCTION();

		for (Ref<Material> m : materials)
		{
			PT_CORE_ASSERT(m->m_PassID >= 0 && m->m_PassID < m_RenderQueue.size(), "Invalid pass ID");
			m_RenderQueue[m->m_PassID].AddJob({ mesh, vertTransformBuf, pixTransformBuf, m });
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

	void Renderer::AddPreRenderCallback(const std::string& pass, std::function<void()> callback)
	{
		int passID = GetPassIDFromName(pass);
		m_PreRenderCallbacks[passID].push_back(callback);
	}

	void Renderer::Render()
	{
		int i = 0;
		for (Pass& p : m_RenderQueue)
		{
			for (auto callback : m_PreRenderCallbacks[i++])
			{
				callback();
			}

			RenderPass(p);
		}
	}

	void Renderer::RenderPass(Pass& pass)
	{
		for (Job& job : pass)
		{
			// Binding the shaders
			job.Material->m_VertexShader->Bind();
			job.Material->m_PixelShader->Bind();

			// Bind all bindables
			job.Mesh->m_IndexBuffer->Bind();
			job.Mesh->m_VertexBuffer->Bind();
			job.Mesh->m_Topology->Bind();

			// Binding transform buffers
			job.VertConstBuf->Bind();
			job.PixConstBuf->Bind();

			for (Ref<Bindable> bind : job.Material->m_Bindables)
				bind->Bind();

			RenderCommand::DrawIndexed(job.Mesh->m_IndexBuffer->size());
		}
	}

	void Renderer::DrawCall(const UINT count)
	{
		PT_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(count);
	}
}