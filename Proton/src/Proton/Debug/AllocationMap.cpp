#include "ptpch.h"
#include "AllocationMap.h"
#include <assert.h>
#include <fstream>

namespace Proton
{
	//std::unordered_map<void*, AllocationMap::AllocationData> AllocationMap::m_AllocationMap = std::unordered_map<void*, AllocationMap::AllocationData>();

	Ref<std::string> AllocationMap::m_CurrentTag = CreateRef<std::string>("");

	void AllocationMap::AddAllocation(void* memAddress, size_t size)
	{
		//assert("Memory location already allocated" && m_AllocationMap.find(memAddress) == m_AllocationMap.end());
		Get().m_AllocationMap.emplace(memAddress, std::move(AllocationData(size, m_CurrentTag)));
	}

	void AllocationMap::RemoveAllocation(void* memAddress)
	{
		assert("Memory location not allocated" && Get().m_AllocationMap.find(memAddress) != Get().m_AllocationMap.end());
		Get().m_AllocationMap.erase(memAddress);
	}

	void AllocationMap::SetTag(const std::string& allocationTag)
	{
		m_CurrentTag = CreateRef<std::string>(allocationTag);
	}

	void AllocationMap::OutputLeaks(const std::string& path)
	{
		std::ofstream stream;
		stream.open(path);

		for (auto& leak : Get().m_AllocationMap)
		{
			stream << "Size: " << leak.second.Size << "Tag: " << *leak.second.AllocationTag << "\n";
		}

		if (Get().m_AllocationMap.empty())
		{
			stream << "No memory leaks!";
		}

		stream.flush();
		stream.close();
	}
}