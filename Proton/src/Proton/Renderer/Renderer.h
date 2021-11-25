#pragma once
#include "RenderCommand.h"
#include "Camera.h"
#include "Proton\Model\Model.h"

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

		static void Submit(class Model& model);

		static void Submit(VertexBuffer* vertBuffer, IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		static void DrawCall(const UINT count);
	private:
		static DirectX::XMMATRIX viewMatrix;
	};
}