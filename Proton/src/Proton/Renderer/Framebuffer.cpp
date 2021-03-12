#include "ptpch.h"
#include "Framebuffer.h"

#include "Platform/DirectX 11/DirectXFramebuffer.h"
#include "Proton\Renderer\Renderer.h"

#include <cassert>

namespace Proton
{
	Ref<Framebuffer> Framebuffer::Create(const FramebufferDescription& desc)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: assert(false && "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::API::DirectX: return CreateRef<DirectXFramebuffer>(desc);
		}

		assert(false && "Unknown Renderer API!");
		return nullptr;
	}
}