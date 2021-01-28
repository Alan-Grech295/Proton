#pragma once
#include "Proton\Renderer\RendererAPI.h"

namespace Proton
{
	class DirectXRendererAPI : public RendererAPI
	{
	public:
		virtual void SetClearColor(float r, float g, float b) override;
		virtual void Clear() override;

		virtual void DrawIndexed(const VertexBuffer* vertBuffer, const IndexBuffer* indexBuffer) override;
	};
}