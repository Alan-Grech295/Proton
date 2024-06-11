#pragma once
#include "RenderCommand.h"
#include "Camera.h"

#include <vector>

namespace Proton
{
	class Pass;

	class Renderer
	{
		friend class SceneRenderer;
		//TEMP
		friend class Scene;
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(const class Mesh* mesh, const std::vector<Ref<class Material>>& materials, class VertexConstantBuffer* vertTransformBuf, class PixelConstantBuffer* pixTransformBuf);

		static void Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static int GetPassIDFromName(const std::string& name);

		static void AddPreRenderCallback(const std::string& pass, std::function<void()> callback);

		static void Render();

		static void RenderPass(Pass& pass);
	private:
		static void DrawCall(const UINT count);
	private:
		static DirectX::XMMATRIX viewMatrix;
		static std::array<Pass, 1> m_RenderQueue;
		static std::array<std::vector<std::function<void()>>, 1> m_PreRenderCallbacks;
	};
}