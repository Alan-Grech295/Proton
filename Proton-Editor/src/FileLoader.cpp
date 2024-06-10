#include "FileLoader.h"
#include "Proton/Asset System/Editor/EditorAssetManager.h"

#include "Proton/Model/Model.h"

namespace Proton
{
    void FileLoader::LoadFile(const std::string& path, DirectX::XMFLOAT3 position, Scene& activeScene)
    {
        EditorAssetManager& assetManager = AssetManager::Instance<EditorAssetManager>();
        if (!assetManager.HasAsset(path)) return;

        Ref<AssetHandle> assetHandle = assetManager.GetAssetHandle(path);

        switch (assetHandle->Type)
        {
        case AssetHandle::Model:
            Ref<Model> model = assetManager.LoadAsset<Model>(path);
            Entity e = Model::CreateEntity(model, activeScene);
            e.GetComponent<TransformComponent>().position = position;
            break;
        }
    }
}

