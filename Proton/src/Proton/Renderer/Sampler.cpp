#include "ptpch.h"
#include "Sampler.h"
#include "Proton\Renderer\Renderer.h"
#include <cassert>
#include "BindsCollection.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXSampler.h"
#endif

namespace Proton
{
	Ref<Sampler> Sampler::Create(const std::string& tag, int slot)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXSampler>(tag, slot);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<Sampler> Sampler::CreateUnique(int slot)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return std::make_unique<DirectXSampler>("", slot);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}
}

