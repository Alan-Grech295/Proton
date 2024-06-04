#pragma once
#include <memory>
#include <string>

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

namespace Proton
{
	//Need to be set
	class CoreUtils
	{
	public:
		static std::string CORE_PATH_STR;
	};

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

	template<typename T, typename P>
	constexpr Ref<T> CastRef(Ref<P> ref)
	{
		return std::static_pointer_cast<T>(ref);
	}

	template<typename T>
	using WeakRef = std::weak_ptr<T>;
}