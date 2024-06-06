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

        PT_CORE_ASSERT(false, "Could not find appropriate asset loader");
        return nullptr;
    }

    void EditorAssetManager::ScanDirectory(const std::filesystem::path& path, bool loadAssets)
    {
        namespace fs = std::filesystem;

        for (const auto& entry : fs::directory_iterator(path))
        {
            if (entry.is_directory())
            {
                ScanDirectory(entry, false);
            }
            // Skipping meta files as they are handled when loading an asset
            else if (entry.path().has_extension() && entry.path().extension() != ".meta")
            {
                HandleFile(entry);
            }
        }

        if (!loadAssets) return;

        // Loads all assets to ensure sub-assets are loaded properly
        LoadAllAssets();

        SaveMetaFiles();
    }

    bool EditorAssetManager::AddOrLoadAssetInternal(const std::filesystem::path& path, AssetHandle::AssetType type, const std::function<Ref<void>()>& loadFunc, Ref<AssetHandle>& outAssetHandle, Ref<void>& outAsset)
    {
        std::filesystem::path relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

        if (pathToUUID.contains(relativePath))
        {
            UUID assetID = pathToUUID[relativePath];
            if (loadedAssets.contains(assetID)) 
            {
                outAssetHandle = uuidToAsset[assetID];
                outAsset = LoadAsset<void>(relativePath);
                return false;
            }
            
            // Asset handle is loaded but actual asset is not loaded
            Ref<void> asset = loadFunc();

            loadedAssets[assetID] = asset;
            outAssetHandle = uuidToAsset[assetID];
            outAsset = asset;
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

        // Meta file already exists so we read it
        if (std::filesystem::exists(path.string() + ".meta"))
        {
            ReadMetaFile(path.string());
            return;
        }

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

    void EditorAssetManager::ReadMetaFile(const std::filesystem::path& path)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(path.string() + ".meta");
        }
        catch (YAML::ParserException e)
        {
            PT_CORE_ERROR("Failed to load .proton file '{0'}\n	{1}", path.string() + ".meta", e.what());
            return;
        }

        ParseMetaData(data, Project::GetAssetRelativePath(path));
    }

    Ref<AssetHandle> EditorAssetManager::ParseMetaData(const YAML::Node& node, const std::filesystem::path& path, bool subAsset)
    {
        UUID assetID = node["ID"].as<UUID>();
        AssetHandle::AssetType assetType = AssetHandle::NameToType(node["Type"].as<std::string>());

        Ref<AssetHandle> assetHandle = CreateRef<AssetHandle>(assetID, assetType);

        if (subAsset)
        {
            assetHandle->SetSubAsset(node["Path"].as<std::string>());
        }

        uuidToAsset.emplace(assetID, assetHandle);

        pathToUUID.emplace(path, assetID);
        uuidToPath.emplace(assetID, path);

        if (node["Sub assets"]) 
        {
            for (const YAML::Node& subNode : node["Sub assets"])
            {
                Ref<AssetHandle> subAssetHandle = ParseMetaData(subNode, path / subNode["Path"].as<std::string>(), true);
                assetHandle->AddSubAsset(subAssetHandle);
            }
        }

        return assetHandle;
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
