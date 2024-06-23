#pragma once
#include "Proton/Model//Model.h"
#include <filesystem>
#include <assimp/scene.h>

namespace Proton
{
    class ModelLoader
    {
    public:
        static Ref<Model> LoadModelEditor(const std::filesystem::path& modelPath);
    private:
        static Ref<Material> DeserializeAssimpMaterial(const std::string& basePath, const aiMaterial& aiMat);
        static Ref<Mesh> DeserializeMesh(const aiMesh& aiMesh, const std::string& modelPath, UUID assetID, const std::vector<Ref<Material>>& materials);
        static Node* DeserializeNode(Node*& outNode, const aiNode& assimpNode, std::vector<Ref<Mesh>>& meshes, uint32_t& index);
    };
}