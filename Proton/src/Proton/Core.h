#pragma once
#include <memory>

#ifdef PT_PLATFORM_WINDOWS
	
#else
	#error Proton only supports windows!
#endif

#define BIT(x) (1 << x)

namespace Proton
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;
}