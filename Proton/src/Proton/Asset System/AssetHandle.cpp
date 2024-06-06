#include "ptpch.h"
#include "AssetHandle.h"

namespace Proton
{
#define X(name) + 1
    std::array<std::string, 0 TYPES> AssetHandle::AssetTypeName = {
        #undef X

#define X(name) #name,
        TYPES
#undef X
    };

    std::unordered_map<std::string, AssetHandle::AssetType> AssetHandle::AssetNameType = {
#define X(name) { #name, AssetType::name },
        TYPES
#undef X
    };
}