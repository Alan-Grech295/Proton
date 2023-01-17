#pragma once
#include <unordered_map>
#include <memory>
#include "Proton\Core\Core.h"

namespace Proton
{
	/*class Mesh;
	class ModelCollection
	{
	public:
		static bool HasMesh(std::string name)
		{
			return Get().meshes.find(name) != Get().meshes.end();
		}

		static void AddMesh(std::string name, Mesh* mesh)
		{
			Get().meshes[name] = mesh;
		}

		static Mesh* GetMesh(std::string name)
		{
			return Get().meshes[name];
		}

		static Mesh* GetMesh(std::string modelPath, std::string meshName)
		{
			return Get().meshes[modelPath + "%" + meshName];
		}

		//Prefabs

		static bool HasPrefabNode(std::string prefabPath, std::string name)
		{
			return Get().prefabNodes.find(prefabPath + "%" + name) != Get().prefabNodes.end();
		}

		static void AddPrefabNode(std::string prefabPath, std::string name, PrefabNode* node)
		{
			Get().prefabNodes[prefabPath + "%" + name] = node;
		}

		static PrefabNode* GetPrefabNode(std::string prefabPath, std::string name)
		{
			return Get().prefabNodes[prefabPath + "%" + name];
		}
	private:

		static ModelCollection& Get()
		{
			static ModelCollection collection;
			return collection;
		}
	private:
		std::unordered_map<std::string, Mesh*> meshes;

		std::unordered_map<std::string, struct PrefabNode*> prefabNodes;
	};*/
}