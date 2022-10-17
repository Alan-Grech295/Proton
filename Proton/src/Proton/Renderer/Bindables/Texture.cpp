#include "ptpch.h"
#include "Texture.h"
#include "BindsCollection.h"
#include "Proton\Asset Loader\AssetManager.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXTexture.h"
#endif

namespace Proton
{
	Ref<Texture2D> Texture2D::Create(std::string path, int slot)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return BindsCollection::Resolve<DirectXTexture2D>(path, slot);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}

	Scope<Texture2D> Texture2D::CreateUnique(std::string path, int slot)
	{
		PT_PROFILE_FUNCTION();

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return CreateScope<DirectXTexture2D>(path, slot);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}
}