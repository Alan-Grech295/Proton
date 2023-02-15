#pragma once

#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Proton\Core\Core.h"

#include "Proton\Debug\Instrumentation.h"

#ifdef PT_PLATFORM_WINDOWS
	#include <Windows.h>
#endif // PT_PLATFORM_WINDOWS

#include "Proton\Core\Base.h"
#include "Proton/Core/Assert.h"

#define PT_PROFILE 1

