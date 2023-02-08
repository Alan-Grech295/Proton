#pragma once
#include "Bindables\Buffer.h"
#include "Bindables\Topology.h"

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
		virtual void Draw(const UINT count) = 0;

		virtual void SetTopology(const TopologyType topology) = 0;
		
		virtual void BindSwapChain() = 0;

		virtual void SetVsync(bool vsync) = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Present() = 0;

		virtual void Initialise(const class Window& window) = 0;

		virtual void EnableBlending() = 0;
		virtual void DisableBlending() = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};
}