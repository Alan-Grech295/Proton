#pragma once
#include "Proton/Model/Mesh.h"
#include "Proton/Renderer/Renderer.h"

namespace Proton
{
	class Mesh;
	class Material;

	class Job
	{
	public:
		Job(Ref<Material> material)
			:
			material(material)
		{}

		virtual void Bind()
		{
			material->m_VertexShader->Bind();
			material->m_PixelShader->Bind();

			for (Ref<Bindable> bind : material->m_Bindables)
				bind->Bind();
		}

		virtual void Draw()
		{
			// Default draw is fullscreen
			RenderCommand::Draw(3);
		}

	public:
		Ref<Material> material;
	};

	class MeshJob : public Job
	{
	public:
		MeshJob(const Mesh* mesh, VertexConstantBuffer* vertConstBuf, PixelConstantBuffer* pixConstBuf, Ref<Material> material)
			:
			Job(material),
			Mesh(mesh),
			VertConstBuf(vertConstBuf),
			PixConstBuf(pixConstBuf)
		{}

		virtual void Bind() override
		{
			Job::Bind();

			// Bind all bindables
			Mesh->m_IndexBuffer->Bind();
			Mesh->m_VertexBuffer->Bind();
			Mesh->m_Topology->Bind();

			// Binding transform buffers
			VertConstBuf->Bind();
			PixConstBuf->Bind();
		}

		virtual void Draw() override
		{
			RenderCommand::DrawIndexed(Mesh->m_IndexBuffer->size());
		}

	public:
		const Mesh* Mesh = nullptr;
		VertexConstantBuffer* VertConstBuf = nullptr;
		PixelConstantBuffer* PixConstBuf = nullptr;
	};
}