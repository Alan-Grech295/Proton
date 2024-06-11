#include "ptpch.h"
#include "MaterialLoader.h"

#include "Proton/Asset System/AssetManager.h"
#include "Proton/Asset System/Editor/EditorAssetManager.h"

namespace Proton
{
    Ref<Material> Proton::MaterialLoader::LoadMaterialEditor(const std::filesystem::path& materialPath)
    {
        EditorAssetManager& assetManager = AssetManager::Instance<EditorAssetManager>();
        Ref<AssetHandle> assetHandle;

        Ref<AssetHandle> modelAssetHandle = assetHandle = assetManager.GetSubAssetHandle(materialPath, AssetHandle::Material);

        while (modelAssetHandle->IsSubAsset() && modelAssetHandle->Type != AssetHandle::Model)
        {
            modelAssetHandle = modelAssetHandle->SubAssetData->ParentAssetHandle;
        }

        PT_CORE_ASSERT(modelAssetHandle && modelAssetHandle->Type == AssetHandle::Model, "Could not find parent model asset for material");

        Ref<Model> modelAsset = AssetManager::LoadAsset<Model>(modelAssetHandle->ID);

        return modelAsset->GetMaterialByName(assetHandle->SubAssetData->RelativePath);
    }
}
