#pragma once
#include "RenderCommand.h"
#include "Camera.h"
#include "Proton\Model\Model.h"

namespace Proton
{
	class Renderer
	{
	public:
		static void BeginScene(Camera& camera);
		static void EndScene();

		static void Submit(class Model& model, DirectX::FXMMATRIX transform);

		static void Submit(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		inline static Camera& GetCamera() { return *m_Camera; }
	private:
		static void DrawCall(const UINT count);
	private:
		static Camera* m_Camera;
	};
}