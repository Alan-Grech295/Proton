#pragma once
#include "Proton/Core/UUID.h"
#include "AssetHandle.h"

namespace Proton
{
    class AssetManager
    {
    public:
        template<typename T>
        Ref<T> LoadAsset(UUID assetID)
        {
            if (loadedAssets.contains(assetID))
                return CastRef<T>(loadedAssets[assetID]);

            Ref<void> assetRef = LoadAssetInternal(assetID);
            loadedAssets[assetID] = assetRef;
            return CastRef<T>(assetRef);
        }

        inline static AssetManager& Instance() { return *instance; }

        template<typename T>
        inline static T& Instance() 
        {
            static_assert(std::is_base_of<AssetManager, T>(), "Trying to cast to non Asset Manager type");

            return *static_cast<T*>(instance);
        }
    protected:
        virtual Ref<void> LoadAssetInternal(UUID assetID) = 0;

        static void SetInstance(AssetManager* manager) 
        {
            PT_CORE_ASSERT(!instance, "Asset manager has already been initialized");
            instance = manager;
        }
    protected:
        std::unordered_map<UUID, Ref<AssetHandle>> uuidToAsset;
        std::unordered_map<UUID, Ref<void>> loadedAssets;
    private:
        static AssetManager* instance;
    };
}