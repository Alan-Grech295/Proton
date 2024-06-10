#pragma once
#include "../AssetManager.h"
#include "Proton/Project/Project.h"

#include <unordered_map>
#include <filesystem>
#include <string>

namespace YAML
{
    class Emitter;
    class Node;
}

namespace Proton
{
    class EditorAssetManager : public AssetManager
    {
#define EXTENSION_TYPE X(".obj", Model) \
                       X(".gltf", Model) \
                       X(".png", Image) \
                       X(".jpg", Image) 

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

        inline bool HasAsset(const std::string& path)
        {
            std::filesystem::path relPath = Project::GetAssetRelativePath(path);
            return pathToUUID.contains(relPath);
        }

        inline Ref<AssetHandle> GetAssetHandle(const std::filesystem::path& path)
        {
            return AssetManager::GetAssetHandle(pathToUUID[Project::GetAssetRelativePath(path)]);
        }

        void ScanDirectory(const std::filesystem::path& path, bool loadAssets = true);        

        template<typename T>
        Ref<T> AddOrLoadSubAsset(const std::filesystem::path& parentPath, const std::string& name, AssetHandle::AssetType type, const std::function<Ref<T>(UUID)>& loadFunc)
        {
            Ref<AssetHandle> assetHandle;
            Ref<void> asset;

            if (AddOrLoadAssetInternal(parentPath / name, type, loadFunc, assetHandle, asset))
            {
                std::filesystem::path relativePath = Project::GetAssetRelativePath(parentPath);
                Ref<AssetHandle> parentAsset = uuidToAsset[pathToUUID[relativePath]];
                assetHandle->SetSubAsset(name, parentAsset);
                parentAsset->AddSubAsset(assetHandle->ID);
            }

            return CastRef<T>(asset);
        }

        inline const std::unordered_map<std::filesystem::path, UUID>& PathToUUID() const
        {
            return pathToUUID;
        }

        inline const std::unordered_map<UUID, std::filesystem::path>& UUIDToPath() const
        {
            return uuidToPath;
        }

    private:
        bool AddOrLoadAssetInternal(const std::filesystem::path& path, AssetHandle::AssetType type, const std::function<Ref<void>(UUID)>& loadFunc, Ref<AssetHandle>& outAssetHandle, Ref<void>& outAsset);

        void HandleFile(const std::filesystem::path& path);
        void SaveMetaFiles();
        void SaveMetaFile(const std::filesystem::path& savePath, Ref<AssetHandle> assetHandle);

        void OutputAsset(YAML::Emitter& out, Ref<AssetHandle> assetHandle);

        void ReadMetaFile(const std::filesystem::path& path);
        Ref<AssetHandle> ParseMetaData(const YAML::Node& node, const std::filesystem::path& path, bool subAsset = false);

        void LoadAllAssets();
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