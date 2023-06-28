#pragma once
#include "Proton/Model/Model.h"
#include <filesystem>
#include <assimp/scene.h>

namespace Proton
{
	class ModelSerializer
	{
	public:
		static Ref<Model> DeserializeModel(const std::filesystem::path& modelPath, UUID uuid);

	private:
		static Ref<Material> DeserializeAssimpMaterial(const std::string& basePath, const aiMaterial& aiMat, UUID uuid);
		static void DeserializeMesh(StaticMesh* outMesh, const aiMesh& mesh, const std::string& modelPath, const std::vector<Ref<Material>>& materials);
		static Node* DeserializeNode(Node*& outNode, const aiNode& assimpNode, std::vector<StaticMesh>& meshes, uint32_t& index);
	};
}