#include "ptpch.h"
#include "RendererAPI.h"

namespace Proton
{
#ifdef  PT_PLATFORM_WINDOWS
	RendererAPI::API RendererAPI::s_API = RendererAPI::API::DirectX;
#endif //  PT_PLATFORM_WINDOWS
}