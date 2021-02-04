#pragma once

#ifdef PT_PLATFORM_WINDOWS
	
#else
	#error Proton only supports windows!
#endif

#define BIT(x) (1 << x)