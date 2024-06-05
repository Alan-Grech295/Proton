#include "ptpch.h"
#include "EditorAssetManager.h"
#include "Proton/Project/Project.h"

// Loaders
#include "Loaders/ModelLoader.h"
#include "Loaders/ImageLoader.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace YAML
{
    template<>
    struct convert<Proton::UUID>
    {
        static Node encode(const Proton::UUID& rhs)
        {

            Node node;
            node.push_back((uint64_t)rhs);
            return node;
        }

        static bool decode(const Node& node, Proton::UUID& rhs)
        {
            rhs = node.as<uint64_t>();
            return true;
        }
    };
}

namespace Proton
{
    Ref<void> EditorAssetManager::LoadAssetInternal(UUID assetID)
    {
        Ref<AssetHandle> handle = uuidToAsset[assetID];
        std::filesystem::path assetPath = Project::GetAssetFileSystemPath(uuidToPath[assetID]);

        switch (handle->Type)
        {
        case AssetHandle::Image:
            return ImageLoader::LoadImageEditor(assetPath);
        case AssetHandle::Model:
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

        // Loads all assets to ensure sub-assets are loaded properly
        LoadAllAssets();

        SaveMetaFiles();
    }

    bool EditorAssetManager::AddOrLoadAssetInternal(const std::filesystem::path& path, AssetHandle::AssetType type, const std::function<Ref<void>()>& loadFunc, Ref<AssetHandle>& outAssetHandle, Ref<void>& outAsset)
    {
        std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

        if (pathToUUID.contains(relativePath))
        {
            outAssetHandle = uuidToAsset[pathToUUID[relativePath]];
            outAsset = LoadAsset<void>(relativePath);
            return false;
        }

        UUID assetID = UUID();
        auto [handle, _] = uuidToAsset.emplace(assetID, CreateRef<AssetHandle>(assetID, type));

        pathToUUID.emplace(relativePath, assetID);
        uuidToPath.emplace(assetID, relativePath);

        Ref<void> asset = loadFunc();

        loadedAssets[assetID] = asset;
        outAssetHandle = handle->second;
        outAsset = asset;
        return true;
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

    void EditorAssetManager::SaveMetaFiles()
    {
        for (auto[uuid, assetHandle] : uuidToAsset) 
        {
            if (assetHandle->IsSubAsset()) continue;

            SaveMetaFile(Project::GetAssetFileSystemPath(uuidToPath[uuid]).string() + ".meta", assetHandle);
        }
    }

    void EditorAssetManager::OutputAsset(YAML::Emitter& out, Ref<AssetHandle> assetHandle)
    {
        out << YAML::BeginMap;
        out << YAML::Key << "ID" << YAML::Value << assetHandle->ID;
        out << YAML::Key << "Type" << YAML::Value << AssetHandle::TypeToName(assetHandle->Type);

        if (assetHandle->IsSubAsset()) 
        {
            out << YAML::Key << "Path" << YAML::Value << assetHandle->SubAssetData->RelativePath;
        }

        if (assetHandle->SubAssets.size() > 0)
        {
            out << YAML::Key << "Sub assets" << YAML::BeginSeq;

            for (UUID assetID : assetHandle->SubAssets)
            {
                auto subAssetHandle = uuidToAsset[assetID];
                OutputAsset(out, subAssetHandle);
            }

            out << YAML::EndSeq;
        }

        out << YAML::EndMap;
    }

    void EditorAssetManager::LoadAllAssets()
    {
        for (auto [uuid, assetHandle] : uuidToAsset) 
        {
            if (assetHandle->IsSubAsset() || loadedAssets.contains(uuid)) continue;

            AssetManager::LoadAsset<void>(uuid);
        }
    }

    void EditorAssetManager::SaveMetaFile(const std::filesystem::path& savePath, Ref<AssetHandle> assetHandle)
    {
        YAML::Emitter out;
        
        OutputAsset(out, assetHandle);

        std::ofstream fout(savePath);
        fout << out.c_str();
    }
}
