#pragma once
#include "RenderCommand.h"
#include "Camera.h"

namespace Proton
{
	class Renderer
	{
		friend class SceneRenderer;
		//TEMP
		friend class Scene;
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(const class StaticMesh* mesh);

		static void Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static int GetPassIDFromName(const std::string& name);

		static void Render();
	private:
		static void DrawCall(const UINT count);
	private:
		static DirectX::XMMATRIX viewMatrix;
		static std::array<class Pass, 1> m_RenderQueue;
	};
}