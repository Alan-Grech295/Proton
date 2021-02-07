#pragma once
#include "Buffer.h"

namespace Proton
{
	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0, DirectX = 1
		};
	public:
		virtual void SetClearColor(float r, float g, float b) = 0;
		virtual void Clear() = 0;

		virtual void DrawIndexed(const UINT count) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}