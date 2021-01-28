#pragma once
#include "RenderCommand.h"

namespace Proton
{
	class Renderer
	{
	public:
		static void BeginScene();
		static void EndScene();

		static void Submit(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer);

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}