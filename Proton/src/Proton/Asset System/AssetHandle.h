#pragma once
#include "Proton/Core/UUID.h"

#include <array>
#include <vector>
#include <string>
#include <unordered_map>

namespace Proton
{
    class AssetHandle
    {
        struct ExtraSubAssetData
        {
        public:
            ExtraSubAssetData(const std::string& relativePath, Ref<AssetHandle> parentAssetHandle)
                : RelativePath(relativePath), ParentAssetHandle(parentAssetHandle)
            {
            }

        public:
            std::string RelativePath;
            Ref<AssetHandle> ParentAssetHandle;
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

        inline static AssetType NameToType(const std::string& name)
        {
            return AssetNameType[name];
        }

        inline void AddSubAsset(UUID assetID)
        {
            SubAssets.push_back(assetID);
        }

        void AddSubAsset(Ref<AssetHandle> asset)
        {
            AddSubAsset(asset->ID);
        }

        inline void SetSubAsset(const std::string& relativePath, Ref<AssetHandle> parentAssetHandle = nullptr)
        {
            SubAssetData = CreateScope<ExtraSubAssetData>(relativePath, parentAssetHandle);
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

        static std::unordered_map<std::string, AssetType> AssetNameType;
    };
}