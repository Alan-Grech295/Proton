#pragma once
#include "RenderCommand.h"
#include "Camera.h"
#include "Proton\Model\Model.h"

namespace Proton
{
	class Renderer
	{
		friend class Scene;
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(class Model& model);

		static void Submit(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

		inline static Camera& GetCamera() { return *m_Camera; }

		inline static void SetCamera(Camera* camera) { m_Camera = camera; }
	private:
		static void DrawCall(const UINT count);
	private:
		static Camera* m_Camera;
		static DirectX::XMMATRIX viewMatrix;
	};
}