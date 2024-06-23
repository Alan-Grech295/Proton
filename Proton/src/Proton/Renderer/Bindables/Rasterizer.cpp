#include "ptpch.h"
#include "Rasterizer.h"
#include "Proton\Renderer\Renderer.h"
#include <cassert>
#include "BindsCollection.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXRasterizer.h"
#endif

namespace Proton
{
	Ref<Rasterizer> Rasterizer::Create(const std::string& tag, bool twoSided)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXRasterizer>(tag, twoSided);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<Rasterizer> Rasterizer::CreateUnique(bool twoSided)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXRasterizer>("", twoSided);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<Rasterizer> Rasterizer::CreateUnique(Ref<Bindable> other)
	{
		assert("Test first before using!" && false);
		return Rasterizer::CreateUnique(dynamic_cast<Rasterizer&>(*other).m_TwoSided);
	}

	Ref<Rasterizer> Rasterizer::Clone(const Rasterizer& other)
	{
		Ref<Rasterizer> clone = std::move(CreateUnique(other.m_TwoSided));
		return clone;
	}
}
