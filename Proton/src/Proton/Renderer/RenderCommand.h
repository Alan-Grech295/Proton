#pragma once
#include "RendererAPI.h"
#include "Proton\Renderer\Bindables\Topology.h"

namespace Proton
{
	class RenderCommand
	{
	public:
		inline static void SetClearColor(float r, float g, float b)
		{
			s_RendererAPI->SetClearColor(r, g, b);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const UINT count)
		{
			s_RendererAPI->DrawIndexed(count);
		}

		inline static void Draw(const UINT count)
		{
			s_RendererAPI->Draw(count);
		}

		inline static void SetTopology(const TopologyType topology)
		{
			s_RendererAPI->SetTopology(topology);
		}

		inline static void BindSwapChain()
		{
			s_RendererAPI->BindSwapChain();
		}

		inline static void SetVsync(bool vsync)
		{
			s_RendererAPI->SetVsync(vsync);
		}

		inline static void Present()
		{
			s_RendererAPI->Present();
		}

		inline static void Resize(uint32_t width, uint32_t height)
		{
			s_RendererAPI->Resize(width, height);
		}

		inline static RendererAPI* GetRendererAPI() { return s_RendererAPI; }
	private:
		static RendererAPI* s_RendererAPI;
	};
}