#include "ptpch.h"
#include "MeshLoader.h"

#include "Proton/Asset System/AssetManager.h"
#include "Proton/Asset System/Editor/EditorAssetManager.h"

namespace Proton
{
    Ref<Mesh> Proton::MeshLoader::LoadMeshEditor(const std::filesystem::path& meshPath)
    {
        EditorAssetManager& assetManager = AssetManager::Instance<EditorAssetManager>();
        Ref<AssetHandle> assetHandle;

        Ref<AssetHandle> modelAssetHandle = assetHandle = assetManager.GetSubAssetHandle(meshPath, AssetHandle::Mesh);

        while (modelAssetHandle->IsSubAsset() && modelAssetHandle->Type != AssetHandle::Model)
        {
            modelAssetHandle = modelAssetHandle->SubAssetData->ParentAssetHandle;
        }

        PT_CORE_ASSERT(modelAssetHandle && modelAssetHandle->Type == AssetHandle::Model, "Could not find parent model asset for mesh");

        Ref<Model> modelAsset = AssetManager::LoadAsset<Model>(modelAssetHandle->ID);

        return modelAsset->GetMeshByName(assetHandle->SubAssetData->RelativePath);
    }
}


