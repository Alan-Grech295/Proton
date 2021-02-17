#include "ptpch.h"
#include "Sampler.h"
#include "Proton\Renderer\Renderer.h"
#include <cassert>

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXSampler.h"
#endif

namespace Proton
{
	Sampler* Sampler::Create(int slot)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXSampler(slot);	
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}
}

