#pragma once
#include "RenderCommand.h"
#include "Camera.h"

namespace Proton
{
	class Renderer
	{
	public:
		static void BeginScene(Camera& camera);
		static void EndScene();

		static void Submit(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		inline static Camera& GetCamera() { return *m_Camera; }
	private:
		static Camera* m_Camera;
	};
}