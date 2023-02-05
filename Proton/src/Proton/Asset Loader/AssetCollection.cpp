#include "ptpch.h"
#include "AssetCollection.h"

namespace Proton
{
	std::unordered_map<std::string, void*> AssetCollection::m_Collection = std::unordered_map<std::string, void*>();
}