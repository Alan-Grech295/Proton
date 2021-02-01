#pragma once
#include "RendererAPI.h"

namespace Proton
{
	class RenderCommand
	{
		friend class WindowsWindow;
		friend class DirectXPixelShader;
		friend class DirectXVertexShader;
	public:
		inline static void SetClearColor(float r, float g, float b)
		{
			s_RendererAPI->SetClearColor(r, g, b);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer)
		{
			s_RendererAPI->DrawIndexed(vertBuffer, indexBuffer);
		}

		inline static RendererAPI* GetRendererAPI() { return s_RendererAPI; }
	private:
		static RendererAPI* s_RendererAPI;
	};
}