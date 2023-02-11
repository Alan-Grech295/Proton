#pragma once
#include <unordered_map>
#include <string>
#include "Proton/Core/Core.h"

namespace Proton
{
	class AllocationMap
	{
	public:
		struct AllocationData
		{
			AllocationData() = default;
			AllocationData(size_t size, Ref<std::string> allocationTag)
				:
				Size(size),
				AllocationTag(allocationTag)
			{}
		public:
			size_t Size;
			Ref<std::string> AllocationTag;
		};

		static void AddAllocation(void* memAddress, size_t size);

		static void RemoveAllocation(void* memAddress);

		static void SetTag(const std::string& allocationTag);

		static void OutputLeaks(const std::string& path = "mem-leaks.output");

		static AllocationMap Get()
		{
			static AllocationMap s_Map = AllocationMap();
			return s_Map;
		}
	private:
		std::unordered_map<void*, AllocationData> m_AllocationMap;
		static Ref<std::string> m_CurrentTag;
	};

	class AllocationScope
	{
		AllocationScope(const std::string& tag)
		{
			AllocationMap::SetTag(tag);
		}

		~AllocationScope()
		{
			AllocationMap::SetTag("");
		}
	};

#define PT_PROFILE 1

#if PT_PROFILE
#define PT_MEM_PROFILE_SCOPE(name)  ::Proton::AllocationScope memScope##__LINE__(name)
#define PT_MEM_PROFILE_FUNCTION(name)  PT_MEM_PROFILE_SCOPE(__FUNCSIG__)
#else
#define PT_MEM_PROFILE_SCOPE(name)
#define PT_MEM_PROFILE_FUNCTION(name)
#endif
}