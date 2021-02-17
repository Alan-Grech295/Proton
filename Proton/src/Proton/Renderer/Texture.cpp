#include "ptpch.h"
#include "Texture.h"

#ifdef PT_PLATFORM_WINDOWS
#include "Platform\DirectX 11\DirectXTexture.h"
#endif

namespace Proton
{
	Texture2D* Texture2D::Create(std::string path, int slot)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			assert("RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::DirectX:
			return new DirectXTexture2D(path, slot);
		}

		assert(false && "Unknown RendererAPI!");
		return nullptr;
	}
}