#include "ptpch.h"
#include "RenderCommand.h"

#include "Platform/DirectX 11/DirectXRendererAPI.h"

namespace Proton
{
	RendererAPI* RenderCommand::s_RendererAPI = new DirectXRendererAPI;
}