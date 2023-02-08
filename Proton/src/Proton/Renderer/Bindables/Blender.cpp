#include "ptpch.h"
#include "Blender.h"
#include "Proton\Renderer\Renderer.h"
#include <cassert>
#include "BindsCollection.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXBlender.h"
#endif

namespace Proton
{
	Ref<Blender> Blender::Create(const std::string& tag, bool blending)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXBlender>(tag, blending);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<Blender> Blender::CreateUnique(bool blending)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXBlender>("", blending);
		}

		assert("Unknown RendererAPI!");
		return nullptr;
	}

	Scope<Blender> Blender::CreateUnique(Ref<Bindable> other)
	{
		assert("Test first before using!" && false);
		return Blender::CreateUnique(dynamic_cast<Blender&>(*other).m_Blending);
	}
}

