#pragma once
#include "Proton\Core\Core.h"
#include <DirectXMath.h>

namespace Proton
{
	struct FramebufferDescription
	{
		uint32_t Width, Height;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual const FramebufferDescription& GetDescription() const = 0;

		virtual void* GetRenderTextureID() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Clear() = 0;
		virtual void SetClearCol(float r, float g, float b) = 0;

		static Ref<Framebuffer> Create(const FramebufferDescription& desc);
	};
}