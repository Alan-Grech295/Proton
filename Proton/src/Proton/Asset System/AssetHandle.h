#pragma once
#include "Proton/Core/UUID.h"

#include <array>
#include <vector>
#include <string>

namespace Proton
{
    class AssetHandle
    {
        struct ExtraSubAssetData
        {
        public:
            ExtraSubAssetData(const std::string& RelativePath)
                : RelativePath(RelativePath)
            {
            }

        public:
            std::string RelativePath;
        };

#define TYPES X(Model) \
              X(Mesh) \
              X(Image) 
    public:
        enum AssetType { 
#define X(name) name,
            TYPES
#undef X
        };

        AssetHandle() = delete;
        AssetHandle(UUID uuid, AssetType type)
            : ID(uuid), Type(type)
        {}

        ~AssetHandle() = default;

        inline static const std::string& TypeToName(AssetType type)
        {
            return AssetTypeName[(int)type];
        }

        void AddSubAsset(UUID assetID)
        {
            SubAssets.push_back(assetID);
        }

        inline void SetSubAsset(const std::string& relativePath)
        {
            SubAssetData = CreateScope<ExtraSubAssetData>(relativePath);
        }

        bool IsSubAsset() const
        {
            return SubAssetData.get();
        }

    public:
        UUID ID;
        AssetType Type;

        std::vector<UUID> SubAssets;
        Scope<ExtraSubAssetData> SubAssetData = nullptr;

    private:
        // Illegal way to count
#define X(name) + 1
        static std::array<std::string, 0 TYPES> AssetTypeName;
#undef X
    };
}