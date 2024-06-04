#include "ptpch.h"
#include "EditorAssetManager.h"
#include "Proton/Project/Project.h"

// Loaders
#include "Loaders/ModelLoader.h"
#include "Loaders/ImageLoader.h"

namespace Proton
{
    Ref<void> EditorAssetManager::LoadAssetInternal(UUID assetID)
    {
        Ref<AssetHandle> handle = uuidToAsset[assetID];
        std::filesystem::path assetPath = Project::GetAssetFileSystemPath(uuidToPath[assetID]);

        switch (handle->Type)
        {
        case AssetHandle::IMAGE:
            return ImageLoader::LoadImageEditor(assetPath);
        case AssetHandle::MODEL:
            return ModelLoader::LoadModelEditor(assetPath);
        }

        PT_CORE_ASSERT(false, "Could not find asset");
        return nullptr;
    }

    void EditorAssetManager::ScanDirectory(const std::filesystem::path& path)
    {
        namespace fs = std::filesystem;

        for (const auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                ScanDirectory(entry);
            }
            else if (entry.path().has_extension())
            {
                HandleFile(entry);
            }
        }
    }
    void EditorAssetManager::HandleFile(const std::filesystem::path& path)
    {
        std::string extension = path.extension().string();
        if (!extensionToType.contains(extension)) return;

        UUID assetID = UUID();
        std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());
        uuidToAsset.emplace(assetID, CreateRef<AssetHandle>(assetID, extensionToType[extension]));

        pathToUUID.emplace(relativePath, assetID);
        uuidToPath.emplace(assetID, relativePath);
    }
}
