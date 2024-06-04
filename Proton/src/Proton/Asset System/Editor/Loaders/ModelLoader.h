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
        static void DeserializeMesh(Mesh* outMesh, const aiMesh& mesh, const std::string& modelPath, Ref<Model> model, const std::vector<Ref<Material>>& materials);
        static Node* DeserializeNode(Node*& outNode, const aiNode& assimpNode, std::vector<Mesh>& meshes, uint32_t& index);
    };
}