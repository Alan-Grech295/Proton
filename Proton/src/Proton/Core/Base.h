#pragma once

#define PT_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

#ifdef PT_DEBUG
#if defined(PT_PLATFORM_WINDOWS)
	#define PT_DEBUGBREAK() __debugbreak()
#elif defined(PT_PLATFORM_LINUX)
	#include <signal.h>
	#define PT_DEBUGBREAK() raise(SIGTRAP)
#else
	#error "Platform doesn't support debugbreak yet!"
#endif
	#define PT_ENABLE_ASSERTS
#else
#define PT_DEBUGBREAK()
#endif

#include "Log.h"