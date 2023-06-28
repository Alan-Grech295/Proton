#include "ptpch.h"
#include "AssetManager.h"
#include "Proton/Core/UUID.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <Platform\DirectX 11\DirectXTexture.h>
#include "Proton\Model\Model.h"

//TEMP
#include "Proton/Renderer/Bindables/DynamicConstantBuffer.h"

#include <yaml-cpp/yaml.h>

namespace fs = std::filesystem;

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
	AssetManager AssetManager::s_Instance = AssetManager();

	AssetManager::AssetManager()
		:
		m_ProjectPath("")
	{
	}

	AssetManager::~AssetManager()
	{
	}

	void AssetManager::ScanProject()
	{
		/*using namespace DCB;
		RawLayout layout;
		layout.Add(DCB::Type::Float, "Float1");
		layout.Add(DCB::Type::Struct, "Structy");
		layout["Structy"].Add(DCB::Type::Array, "Array");
		layout["Structy"]["Array"].Set(DCB::Type::Int, 4);
		layout["Structy"].Add(DCB::Type::Bool, "Booly");
		layout.Add(DCB::Type::Array, "Array2");
		layout["Array2"].Set(DCB::Type::Array, 2);
		layout["Array2"].T().Set(DCB::Type::Float, 2);

		Buffer buffer(std::move(layout));

		buffer["Float1"] = 2.0f;
		float float1 = (float)buffer["Float1"];

		buffer["Structy"]["Booly"] = true;
		bool booly2 = (bool)buffer["Structy"]["Booly"];

		buffer["Array2"][0][0] = 10.0f;
		buffer["Array2"][0][1] = 20.0f;
		buffer["Array2"][1][0] = 13.0f;
		buffer["Array2"][1][1] = -10.0f;

		float a = (float)buffer["Array2"][0][0];
		float b = (float)buffer["Array2"][0][1];
		float c = (float)buffer["Array2"][1][0];
		float d = (float)buffer["Array2"][1][1];*/

		s_Instance.ScanDirectory(s_Instance.m_ProjectPath.string());
	}

	void AssetManager::SetProjectPath(const std::filesystem::path path)
	{
		s_Instance.m_ProjectPath = path;
	}

	void AssetManager::AddAsset(UUID uuid, Ref<Asset> asset)
	{
		s_Instance.m_LoadedAssets.emplace(uuid, asset);
	}

	/*template<typename T>
	Ref<T> AssetManager::GetAsset(UUID uuid)
	{
		return static_pointer_cast<T>(s_Instance.m_LoadedAssets.at(uuid));
	}

	template<typename T>
	Ref<T> AssetManager::GetAsset(const std::filesystem::path& filepath)
	{
		return GetAsset(s_Instance.m_PathToUUID.at(filepath));
	}*/

	void AssetManager::ScanDirectory(const std::filesystem::path& path)
	{
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

	void AssetManager::HandleFile(const std::filesystem::path& path)
	{
		if (std::filesystem::exists(path.string() + ".meta"))
		{
			// Reading meta file
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

			UUID uuid = data["UUID"].as<UUID>();
			Asset::AssetType type = (Asset::AssetType)data["Importer"].as<uint32_t>();
			RegisterEditorAsset(path, uuid, type);
			return;
		}

		// Creates the meta file
		if (m_AssetTypes.find(path.extension().string()) != m_AssetTypes.end()
			&& m_AssetTypes[path.extension().string()] == Asset::Model)
		{ 
			UUID uuid = UUID();

			YAML::Emitter out; 
			out << YAML::BeginMap; 
			
			out << YAML::Key << "UUID" << YAML::Value << uuid;
			out << YAML::Key << "Importer" << YAML::Value << (uint32_t)Asset::Model;
			
			out << YAML::Key << "ImporterSettings" << YAML::Value; 
			{ 
				out << YAML::BeginMap; 
				out << YAML::EndMap; 
			} 
			
			out << YAML::EndMap; 
			
			std::ofstream fout(path.string() + ".meta"); 
			fout << out.c_str(); 

			RegisterEditorAsset(path, uuid, Asset::Model);
		}
	}
	void AssetManager::RegisterEditorAsset(const std::filesystem::path& path, UUID uuid, Asset::AssetType type)
	{
		// Asset was already loaded
		if (m_AssetRegistry.find(uuid) != m_AssetRegistry.end())
			return;

		m_AssetRegistry.emplace(uuid, type);
		m_PathToUUID[path] = uuid;
		m_UUIDToPath[uuid] = path;
	}
}
