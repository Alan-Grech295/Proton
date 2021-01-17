#include "ptpch.h"
#include "Proton/Drawable/Drawable.h"
#include "Proton/Bindable/IndexBuffer.h"
#include <cassert>
#include <typeinfo>
#include "Proton/Log.h"

namespace Proton
{
	void Drawable::Draw(WindowsGraphics& gfx) const noexcept
	{
		for (auto& b : binds)
		{
			b->Bind(gfx);
		}

		for (auto& b : GetStaticBinds())
		{
			b->Bind(gfx);
		}

		//PT_CORE_INFO(pIndexBuffer->GetCount());
		gfx.DrawIndexed(pIndexBuffer->GetCount());
	}

	void Drawable::AddBind(std::unique_ptr<Bindable> bind) noexcept
	{
		assert("*Must* use AddIndexBuffer to bind index buffer" && typeid(*bind) != typeid(IndexBuffer));
		binds.push_back(std::move(bind));
	}

	void Drawable::AddIndexBuffer(std::unique_ptr<IndexBuffer> ibuf) noexcept
	{
		assert("Attempting to add index buffer a second time" && pIndexBuffer == nullptr);
		pIndexBuffer = ibuf.get();
		binds.push_back(std::move(ibuf));
	}
}