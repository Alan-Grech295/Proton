#pragma once
#include "Asset.h"

#include <unordered_map>
#include <filesystem>
#include <vector>
#include <type_traits>

namespace Proton
{
	class Entity;

	struct path_hash
	{
		std::size_t operator() (const std::filesystem::path& path) const
		{
			std::hash<std::string> hasher;
			return hasher(path.string());
		}
	};

	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();

		static void ScanProject();
		static void SetProjectPath(const std::filesystem::path path);

		static void AddAsset(UUID uuid, Ref<Asset> asset);
		template<typename T>
		inline static Ref<T> GetEditorAsset(const std::filesystem::path& path, UUID uuid)
		{
			if (s_Instance.m_LoadedAssets.find(uuid) == s_Instance.m_LoadedAssets.end())
			{
				Ref<T> asset = T::DeserializeEditor(path, uuid);
				s_Instance.m_LoadedAssets[uuid] = asset;
				return asset;
			}
			
			return std::static_pointer_cast<T>(s_Instance.m_LoadedAssets.at(uuid));
		}

		// Editor only
		template<typename T>
		inline static Ref<T> GetEditorAsset(const std::filesystem::path& path)
		{
			return GetEditorAsset<T>(path, s_Instance.m_PathToUUID.at(path));
		}

		template<typename T>
		inline static Ref<T> GetEditorAsset(UUID uuid)
		{
			return GetEditorAsset<T>(s_Instance.m_UUIDToPath.at(uuid), uuid);
		}
	private:
		void ScanDirectory(const std::filesystem::path& path);
		void HandleFile(const std::filesystem::path& path);
		void RegisterEditorAsset(const std::filesystem::path& path, UUID uuid, Asset::AssetType type);
	private:
		std::filesystem::path m_ProjectPath;
		std::unordered_map<std::string, Asset::AssetType> m_AssetTypes = {
			{".obj", Asset::AssetType::Model},
			{".gltf", Asset::AssetType::Model},
			{".blend", Asset::AssetType::Model},
		};

		std::unordered_map<UUID, Ref<Asset>> m_LoadedAssets;

		std::unordered_map<std::filesystem::path, UUID> m_PathToUUID;
		std::unordered_map<UUID, std::filesystem::path> m_UUIDToPath;
		std::unordered_map<UUID, AssetMetaData> m_AssetRegistry;

		static AssetManager s_Instance;
	};
}