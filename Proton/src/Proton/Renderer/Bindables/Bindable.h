#pragma once

namespace Proton
{
	class Bindable
	{
	public:
		virtual void Bind() = 0;
		virtual void Unbind() {}

		virtual Ref<Bindable> Clone() = 0;
	};
}