#pragma once
#include "Log.h"
#include "Base.h"

#define PT_ENABLE_ASSERTS

#ifdef PT_ENABLE_ASSERTS
	#define PT_EXPAND_MACRO(x) x
	#define PT_STRINGIFY_MACRO(x) #x
	// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define PT_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { PT_##type##_ERROR(msg, __VA_ARGS__); PT_DEBUGBREAK(); } }
	#define PT_INTERNAL_ASSERT_WITH_MSG(type, check, ...) PT_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define PT_INTERNAL_ASSERT_NO_MSG(type, check) PT_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", PT_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
			
	#define PT_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define PT_INTERNAL_ASSERT_GET_MACRO(...) PT_EXPAND_MACRO( PT_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, PT_INTERNAL_ASSERT_WITH_MSG, PT_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define PT_ASSERT(...) PT_EXPAND_MACRO( PT_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define PT_CORE_ASSERT(...) PT_EXPAND_MACRO( PT_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(CORE, __VA_ARGS__) )
#else
	#define PT_ASSERT(...)
	#define PT_CORE_ASSERT(...)
#endif