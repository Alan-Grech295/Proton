#pragma once
#include "RenderCommand.h"
#include "Camera.h"
#include "Proton\Renderer\Render Queue\Pass.h"

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

		static void Submit(class Mesh& mesh);
		static void Submit(class Mesh* mesh);

		static void Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		inline static int GetPassIDFromName(const std::string& name)
		{
			for (int i = 0; i < m_RenderQueue.size(); i++)
			{
				if (m_RenderQueue[i].m_Name == name)
					return i;
			}

			assert("Pass name not found!" && false);
			return -1;
		}

		static void Render();
	private:
		static void DrawCall(const UINT count);
	private:
		static DirectX::XMMATRIX viewMatrix;
		static std::array<Pass, 1> m_RenderQueue;
	};
}