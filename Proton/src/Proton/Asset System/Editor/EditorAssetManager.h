#pragma once
#include "../AssetManager.h"
#include "Proton/Project/Project.h"

#include <unordered_map>
#include <filesystem>
#include <string>

namespace Proton
{
    class EditorAssetManager : public AssetManager
    {
#define EXTENSION_TYPE X(".obj", MODEL) \
                       X(".gltf", MODEL) \
                       X(".png", IMAGE) \
                       X(".jpg", IMAGE) 

    public:
        EditorAssetManager() 
        {
            AssetManager::SetInstance(this);
        }
    public:
        template<typename T>
        inline Ref<T> LoadAsset(const std::filesystem::path& path)
        {
            std::filesystem::path relPath = Project::GetAssetRelativePath(path);
            PT_CORE_ASSERT(pathToUUID.contains(relPath), "Asset not found")
            return AssetManager::LoadAsset<T>(pathToUUID[relPath]);
        }

        template<typename T>
        inline Ref<T> LoadAsset(const std::string& path)
        {
            return LoadAsset<T>(std::filesystem::path(path));
        }

        template<typename T>
        inline Ref<T> LoadAsset(const char* path)
        {
            return LoadAsset<T>(std::filesystem::path(path));
        }

        void ScanDirectory(const std::filesystem::path& path);
    private:
        void HandleFile(const std::filesystem::path& path);
    protected:
        Ref<void> LoadAssetInternal(UUID assetID) override;
    private:
        std::unordered_map<std::filesystem::path, UUID> pathToUUID;
        std::unordered_map<UUID, std::filesystem::path> uuidToPath;

        std::unordered_map<std::string, AssetHandle::AssetType> extensionToType = {
#define X(ext, type) { ext, AssetHandle::AssetType::type },
            EXTENSION_TYPE
#undef X
        };
    };
}