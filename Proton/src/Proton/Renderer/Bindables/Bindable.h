#pragma once

namespace Proton
{
	class Bindable
	{
	public:
		virtual void Bind() = 0;

		virtual Ref<Bindable> Clone() = 0;
	};
}