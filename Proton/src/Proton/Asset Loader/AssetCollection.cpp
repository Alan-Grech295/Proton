#include "ptpch.h"
#include "AssetCollection.h"

namespace Proton
{
	std::unordered_map<std::string, Ref<void>> AssetCollection::m_Collection = std::unordered_map<std::string, Ref<void>>();
}