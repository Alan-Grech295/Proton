#pragma once
#include "RenderCommand.h"
#include "Camera.h"

#include <vector>
#include <unordered_map>

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
		static void Submit(const class Mesh* mesh, Ref<class Material> material, class VertexConstantBuffer* vertTransformBuf, class PixelConstantBuffer* pixTransformBuf);

		static void SubmitFullScreen(const std::vector<Ref<class Material>>& materials);
		static void SubmitFullScreen(Ref<class Material> material);

		static void Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		static int GetPassIDFromName(const std::string& name);

		static void AddPreRenderCallback(const std::string& pass, std::function<void()> callback);

		static void Render();

		static void RenderPass(Pass& pass);

		static int AddPass(const std::string& name);
		static int AddPassBefore(const std::string& name, const std::string& beforePassName);
		static int AddPassAfter(const std::string& name, const std::string& afterPassName);
	private:
		static void DrawCall(const UINT count);
	private:
		static DirectX::XMMATRIX viewMatrix;
		static std::vector<Pass> m_RenderQueue;
		static std::unordered_map<int, std::vector<std::function<void()>>> m_PreRenderCallbacks;
		static int NextPassID;
	};
}