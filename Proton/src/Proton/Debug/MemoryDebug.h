#pragma once
#include "AllocationMap.h"

#if PT_PROFILE
#define OVERRIDE_NEW void* operator new(size_t size) \
{\
	void* mem = malloc(size);\
	Proton::AllocationMap::AddAllocation(mem, size);\
	return mem;\
}\
\
void operator delete(void* ptr)\
{\
	Proton::AllocationMap::RemoveAllocation(ptr);\
	free(ptr);\
}
#endif