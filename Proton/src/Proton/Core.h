#pragma once

#ifdef PT_PLATFORM_WINDOWS
	#ifdef PT_BUILD_DLL
		#define PROTON_API __declspec(dllexport)
	#else
		#define PROTON_API __declspec(dllimport)
	#endif
#else
	#error Proton only supports windows!
#endif

#define BIT(x) (1 << x)