#pragma once
#include <memory>

#ifdef PT_PLATFORM_WINDOWS
#define DX_CHECK_ERROR(x)   { \
								HRESULT hr = x; \
								if (FAILED(hr)) \
								{ \
								GET_ERROR(hr); \
								} \
							}
#else
	#error Proton only supports windows!
#endif

#define BIT(x) (1 << x)

#define CORE_PATH "C:\\Dev\\Proton"
#define CORE_PATH_STR std::string(CORE_PATH)

namespace Proton
{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}