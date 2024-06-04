#include "ptpch.h"
#include "ModelLoader.h"
#include "../../AssetManager.h"
#include "../EditorAssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <stack>

namespace Proton
{
    static int CountAssimpNodes(aiNode* rootNode) 
    {
        // Node deserialization
        std::stack<aiNode*> nodes;

        // Counting the nodes
        int count = 0;
        nodes.emplace(rootNode);
        while (!nodes.empty())
        {
            aiNode* node = nodes.top();
            nodes.pop();
            count++;

            for (int i = 0; i < node->mNumChildren; i++)
                nodes.emplace(node->mChildren[i]);
        }

        return count;
    }

    Ref<Model> ModelLoader::LoadModelEditor(const std::filesystem::path& modelPath)
    {
        Assimp::Importer imp;
        const auto pScene = imp.ReadFile(modelPath.string().c_str(),
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_ConvertToLeftHanded |
            aiProcess_GenNormals |
            aiProcess_CalcTangentSpace);

        Ref<Model> model = CreateRef<Model>();

        std::string basePath = std::filesystem::path(modelPath).remove_filename().string();

        if (strlen(imp.GetErrorString()) > 0)
        {
            PT_CORE_ERROR("Error importing model: {0}", std::string(imp.GetErrorString()));
            return model;
        }

        aiNode& node = *pScene->mRootNode;

        model->m_Meshes.resize(pScene->mNumMeshes);
        std::vector<Ref<Material>> materials;
        materials.resize(pScene->mNumMaterials);

        // Material deserialization
        for (int i = 0; i < pScene->mNumMaterials; i++)
        {
            //UUID materialUUID = UUID();
            Ref<Material> mat = DeserializeAssimpMaterial(basePath, *pScene->mMaterials[i]);
            materials[i] = mat;
            //AssetManager::AddAsset(materialUUID, mat);
        }

        // Mesh deserialization
        for (int i = 0; i < pScene->mNumMeshes; i++)
        {
            DeserializeMesh(&model->m_Meshes[i], *pScene->mMeshes[i], modelPath.string(), model, materials);
        }

        int nodeCount = CountAssimpNodes(pScene->mRootNode);
        
        // Serializing the nodes
        model->m_Nodes.resize(nodeCount);
        uint32_t index = 0;
        Node* nextNode = model->m_Nodes.data();
        DeserializeNode(nextNode, *pScene->mRootNode, model->m_Meshes, index);

        return model;
    }

    Ref<Material> ModelLoader::DeserializeAssimpMaterial(const std::string& basePath, const aiMaterial& aiMat)
    {
        namespace dx = DirectX;

        // Single pass
        Ref<Material> material = CreateRef<Material>();

        Ref<Material::Pass> opaque = CreateRef<Material::Pass>("Opaque");

        float shininess = 40.0f;
        bool hasAlphaGloss = false;
        bool hasAlphaDiffuse = false;
        dx::XMFLOAT4 diffuseColor = { 1.0f,0.0f,1.0f,1.0f };
        dx::XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };

        aiString texFileName;

        EditorAssetManager& assetManager = AssetManager::Instance<EditorAssetManager>();

        // Diffuse map
        if (aiMat.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
        {
            material->hasDiffuseMap = true;
            Ref<Texture2D> diffuse = assetManager.LoadAsset<Texture2D>(basePath + texFileName.C_Str());
            diffuse->SetSlot(0);
            opaque->AddBindable(diffuse);
            hasAlphaDiffuse = diffuse->HasAlpha();
        }
        else
        {
            aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
        }

        // Specular map
        if (aiMat.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
        {
            material->hasSpecular = true;
            Ref<Texture2D> specular = assetManager.LoadAsset<Texture2D>(basePath + texFileName.C_Str());
            specular->SetSlot(1);
            opaque->AddBindable(specular);
            hasAlphaGloss = specular->HasAlpha();
        }
        else
        {
            aiMat.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
        }

        if (!hasAlphaGloss)
        {
            aiMat.Get(AI_MATKEY_SHININESS, shininess);
        }

        // Normal map
        if (aiMat.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
        {
            material->hasNormalMap = true;
            Ref<Texture2D> normal = assetManager.LoadAsset<Texture2D>(basePath + texFileName.C_Str());
            normal->SetSlot(2);
            opaque->AddBindable(normal);
        }

        // Sampler
        if (material->hasSpecular || material->hasNormalMap || material->hasDiffuseMap)
        {
            opaque->AddBindable(Sampler::Create(basePath + "mat_" + aiMat.GetName().C_Str()));
        }

        //Blender code if it needs to be added
        /*Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, basePath + "mat_" + std::to_string(index));
        blender->Initialize<Blender>(blending);
        step.AddBindable(blender);
        });//*/

        //Rasterizer
        opaque->AddBindable(Rasterizer::Create(basePath + "mat_" + aiMat.GetName().C_Str(), hasAlphaDiffuse));

        // Shaders
        std::string pixShaderPath;
        std::string vertShaderPath;

        if (material->hasSpecular && !material->hasNormalMap)
        {
            pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongSpecularPS.cso";
            vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapVS.cso";
        }
        else if (material->hasNormalMap && !material->hasSpecular)
        {
            pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapPS.cso";
            vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapVS.cso";
        }
        else if (material->hasNormalMap && material->hasSpecular)
        {
            pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapSpecPS.cso";
            vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapVS.cso";
        }
        else if (!material->hasNormalMap && !material->hasSpecular && material->hasDiffuseMap)
        {
            pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongPS.cso";
            vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongVS.cso";
        }
        else
        {
            pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNoTexPS.cso";
            vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongVS.cso";
        }

        opaque->m_PixelShader = PixelShader::Create(pixShaderPath);
        opaque->m_VertexShader = VertexShader::Create(vertShaderPath);

        // TODO: Convert pixel constant buffer creation to reflect
        // pixel shader (i.e. pixel shader is chosen first then the 
        // pixel constant buffer is created)
        if (!(material->hasDiffuseMap || material->hasSpecular || material->hasNormalMap))
        {
            DCB::RawLayout layout;
            layout.Add(DCB::Type::Float4, "materialColor");
            layout.Add(DCB::Type::Float4, "specularColor");
            layout.Add(DCB::Type::Float, "specularPower");

            Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));

            (*pcb)["materialColor"] = (dx::XMFLOAT4)diffuseColor;
            (*pcb)["specularColor"] = (dx::XMFLOAT4)specularColor;
            (*pcb)["specularPower"] = (float)shininess;

            opaque->AddBindable(pcb);
        }
        else
        {
            DCB::RawLayout layout;
            layout.Add(DCB::Type::Float, "specularIntensity");
            layout.Add(DCB::Type::Float, "specularPower");
            layout.Add(DCB::Type::Bool, "hasAlphaGloss");

            Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));

            (*pcb)["specularIntensity"] = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
            (*pcb)["specularPower"] = shininess;
            (*pcb)["hasAlphaGloss"] = hasAlphaGloss;

            opaque->AddBindable(pcb);
        }

        material->AddPass(opaque);

        return material;
    }

    void ModelLoader::DeserializeMesh(Mesh* outMesh, const aiMesh& mesh, const std::string& modelPath, Ref<Model> model, const std::vector<Ref<Material>>& materials)
    {
        namespace dx = DirectX;

        using namespace std::string_literals;

        auto meshTag = modelPath + "%" + mesh.mName.C_Str();

        *outMesh = Mesh(model);

        outMesh->m_Name = mesh.mName.C_Str();

        outMesh->material = materials[mesh.mMaterialIndex];

        BufferLayout layout = {
            {"POSITION", ShaderDataType::Float3},
            {"NORMAL", ShaderDataType::Float3},
            {"TANGENT", ShaderDataType::Float3},
            {"BITANGENT", ShaderDataType::Float3},
            {"TEXCOORD", ShaderDataType::Float2}
        };

        VertexShader* vertShader = outMesh->material->m_Passes[0]->m_VertexShader.get();
        Ref<VertexBuffer> vertBuffer = VertexBuffer::Create(meshTag, layout, vertShader, mesh.mNumVertices);

        {
            PT_PROFILE_SCOPE("Vertex Loading - Model::SerializeMesh");
            for (unsigned int i = 0; i < mesh.mNumVertices; i++)
            {
                dx::XMFLOAT3& position = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]);
                dx::XMFLOAT3& normal = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]);
                dx::XMFLOAT3& tangent = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]);
                dx::XMFLOAT3& bitangent = *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]);
                dx::XMFLOAT2& texcoord = *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i]);

                vertBuffer->EmplaceBack(
                    position,
                    normal,
                    tangent,
                    bitangent,
                    texcoord
                );
            }
        }

        outMesh->m_VertexBuffer = vertBuffer;

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(meshTag, mesh.mNumFaces * 3);

        {
            PT_PROFILE_SCOPE("Index Loading - Model::SerializeMesh");
            for (unsigned int i = 0; i < mesh.mNumFaces; i++)
            {
                const auto& face = mesh.mFaces[i];
                assert(face.mNumIndices == 3);
                indexBuffer->EmplaceBack(face.mIndices[0]);
                indexBuffer->EmplaceBack(face.mIndices[1]);
                indexBuffer->EmplaceBack(face.mIndices[2]);
            }
        }

        outMesh->m_IndexBuffer = indexBuffer;
    }

    Node* ModelLoader::DeserializeNode(Node*& outNode, const aiNode& assimpNode, std::vector<Mesh>& meshes, uint32_t& index)
    {
        Node* curNode = outNode;
        *curNode = Node();
        curNode->m_Name = assimpNode.mName.C_Str();

        curNode->m_Transformation = DirectX::XMMatrixTranspose(
            DirectX::XMLoadFloat4x4(
                reinterpret_cast<const DirectX::XMFLOAT4X4*>(&assimpNode.mTransformation)
            )
        );

        curNode->m_Meshes.reserve(assimpNode.mNumMeshes);
        for (int i = 0; i < assimpNode.mNumMeshes; i++)
        {
            curNode->m_Meshes.push_back(&meshes[assimpNode.mMeshes[i]]);
        }

        curNode->m_Children.reserve(assimpNode.mNumChildren);
        for (int i = 0; i < assimpNode.mNumChildren; i++)
        {
            index++;
            outNode++;
            curNode->m_Children.push_back(DeserializeNode(outNode, *assimpNode.mChildren[i], meshes, index));
        }

        return curNode;
    }
}
