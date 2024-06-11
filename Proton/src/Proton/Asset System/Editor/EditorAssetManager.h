#pragma once
#include "../AssetManager.h"
#include "Proton/Project/Project.h"

#include "Proton/Model/Model.h"
#include "Proton/Model/Mesh.h"
#include "Proton/Model/Material.h"

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

#define ASSET_TYPE X(Model, Model) \
X(Mesh, Mesh) \
X(Material, Material) \
X(Image, Texture2D) 

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
        inline Ref<T> LoadSubAsset(const std::filesystem::path& path)
        {
            std::filesystem::path relPath = GetSubAssetPath<T>(path);
            PT_CORE_ASSERT(pathToUUID.contains(relPath), "Asset not found")
            return AssetManager::LoadAsset<T>(pathToUUID[relPath]);
        }

        inline std::filesystem::path GetSubAssetPath(AssetHandle::AssetType type, const std::filesystem::path& path) const
        {
            std::filesystem::path result = Project::GetAssetRelativePath(path);
            switch (type)
            {
#define X(name) case AssetHandle::name: result = result.string() + "_" + #name; break;
                TYPES
#undef X
            }

            return result;
        }

        template<typename T>
        inline std::filesystem::path GetSubAssetPath(const std::filesystem::path& path) const
        {
#define X(handleType, type) if constexpr (std::is_same<T, type>::value) { return GetSubAssetPath(AssetHandle::handleType, path); }
            ASSET_TYPE
#undef X
        }

        inline bool HasAsset(const std::string& path)
        {
            std::filesystem::path relPath = Project::GetAssetRelativePath(path);
            return pathToUUID.contains(relPath);
        }

        inline bool HasSubAsset(const std::string& path, AssetHandle::AssetType type)
        {
            std::filesystem::path relPath = GetSubAssetPath(type, path);
            return pathToUUID.contains(relPath);
        }

        inline Ref<AssetHandle> GetAssetHandle(const std::filesystem::path& path)
        {
            return AssetManager::GetAssetHandle(pathToUUID[Project::GetAssetRelativePath(path)]);
        }

        inline Ref<AssetHandle> GetSubAssetHandle(const std::filesystem::path& path, AssetHandle::AssetType type)
        {
            return AssetManager::GetAssetHandle(pathToUUID[GetSubAssetPath(type, path)]);
        }

        void ScanDirectory(const std::filesystem::path& path, bool loadAssets = true);        

        template<typename T>
        Ref<T> AddOrLoadSubAsset(const std::filesystem::path& parentPath, const std::string& name, AssetHandle::AssetType type, const std::function<Ref<T>(UUID)>& loadFunc)
        {
            Ref<AssetHandle> assetHandle;
            Ref<void> asset;

            if (AddOrLoadAssetInternal(GetSubAssetPath<T>(parentPath / name), type, loadFunc, assetHandle, asset))
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
        bool AddOrLoadAssetInternal(const std::filesystem::path& relativePath, AssetHandle::AssetType type, const std::function<Ref<void>(UUID)>& loadFunc, Ref<AssetHandle>& outAssetHandle, Ref<void>& outAsset);

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