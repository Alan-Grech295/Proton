#include "ptpch.h"
#include "Renderer.h"
#include "Proton\Model\Model.h"
#include "Proton/Core/Core.h"
#include "Proton\Renderer\Render Queue\Pass.h"

namespace Proton
{
	std::vector<Pass> Renderer::m_RenderQueue = {Pass("Opaque", 0)};
	int Renderer::NextPassID = 1;
	std::unordered_map<int, std::vector<std::function<void()>>> Renderer::m_PreRenderCallbacks = {};

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
#if PT_DEBUG
			bool foundPass = false;
			for (const Pass& p : m_RenderQueue)
			{
				if (p.m_PassID == m->m_PassID)
				{
					foundPass = true;
					break;
				}
			}
			PT_CORE_ASSERT(foundPass, "Invalid pass ID");
#endif
			m_RenderQueue[m->m_PassID].AddJob(CreateRef<MeshJob>(mesh, vertTransformBuf, pixTransformBuf, m));
		}
	}

	void Renderer::Submit(const Mesh* mesh, Ref<class Material> material, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf)
	{
		PT_PROFILE_FUNCTION();

#if PT_DEBUG
		bool foundPass = false;
		for (const Pass& p : m_RenderQueue)
		{
			if (p.m_PassID == material->m_PassID)
			{
				foundPass = true;
				break;
			}
		}
		PT_CORE_ASSERT(foundPass, "Invalid pass ID");
#endif
		m_RenderQueue[material->m_PassID].AddJob(CreateRef<MeshJob>(mesh, vertTransformBuf, pixTransformBuf, material));
	}

	void Renderer::SubmitFullScreen(const std::vector<Ref<Material>>& materials)
	{
		for (Ref<Material> mat : materials)
			SubmitFullScreen(mat);
	}

	void Renderer::SubmitFullScreen(Ref<Material> material)
	{
		m_RenderQueue[material->m_PassID].AddJob(CreateRef<Job>(material));
	}

	void Renderer::Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer)
	{
		vertBuffer->Bind();
		indexBuffer->Bind();
		RenderCommand::DrawIndexed(indexBuffer->size());
	}

	int Renderer::GetPassIDFromName(const std::string& name)
	{
		for (const Pass& p : m_RenderQueue)
		{
			if (p.m_Name == name)
				return p.m_PassID;
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
		for (Ref<Job> job : pass)
		{
			// Binding the shaders
			job->Bind();

			// Draw the job
			job->Draw();
		}
	}

	int Renderer::AddPass(const std::string& name)
	{
		return m_RenderQueue.emplace_back(name, NextPassID++).m_PassID;
	}

	int Renderer::AddPassBefore(const std::string& name, const std::string& beforePassName)
	{
		auto insertPassIt = m_RenderQueue.end();

		for (auto it = m_RenderQueue.begin(); it != m_RenderQueue.end(); ++it)
		{
			if (it->m_Name == beforePassName)
			{
				insertPassIt = it;
				break;
			}
		}

		if (insertPassIt == m_RenderQueue.end())
		{
			PT_CORE_ERROR("Could not find pass with name {0}", beforePassName);
			return AddPass(name);
		}

		int passID = NextPassID++;
		return m_RenderQueue.insert(insertPassIt, { name, NextPassID++ })->m_PassID;
	}

	int Renderer::AddPassAfter(const std::string& name, const std::string& afterPassName)
	{
		auto insertPassIt = m_RenderQueue.end();

		for (auto it = m_RenderQueue.begin(); it != m_RenderQueue.end(); ++it)
		{
			if (it->m_Name == afterPassName)
			{
				insertPassIt = ++it;
				break;
			}
		}

		if (insertPassIt == m_RenderQueue.end())
		{
			PT_CORE_ERROR("Could not find pass with name {0}", afterPassName);
			return AddPass(name);
		}

		int passID = NextPassID++;
		return m_RenderQueue.insert(insertPassIt, { name, NextPassID++ })->m_PassID;
	}

	void Renderer::DrawCall(const UINT count)
	{
		PT_PROFILE_FUNCTION();

		RenderCommand::DrawIndexed(count);
	}
}