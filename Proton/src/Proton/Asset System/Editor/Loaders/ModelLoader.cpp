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

        model->m_Meshes.reserve(pScene->mNumMeshes);
        model->m_DefaultMaterials.reserve(pScene->mNumMaterials);

        EditorAssetManager& assetManager = AssetManager::Instance<EditorAssetManager>();

        // Material deserialization
        for (int i = 0; i < pScene->mNumMaterials; i++)
        {
            //UUID materialUUID = UUID();
            model->m_DefaultMaterials.push_back(assetManager.AddOrLoadSubAsset<Material>(modelPath, pScene->mMaterials[i]->GetName().C_Str(),
                AssetHandle::Material, [pScene, i, basePath](UUID assetID)
                {
                    return DeserializeAssimpMaterial(basePath, *pScene->mMaterials[i], assetID);
                })
            );
            //AssetManager::AddAsset(materialUUID, mat);
        }

        // Mesh deserialization
        for (int i = 0; i < pScene->mNumMeshes; i++)
        {
            model->m_Meshes.push_back(
                assetManager.AddOrLoadSubAsset<Mesh>(modelPath, pScene->mMeshes[i]->mName.C_Str(), 
                    AssetHandle::Mesh, [pScene, i, modelPath, model](UUID assetID)
                    { 
                        return DeserializeMesh(*pScene->mMeshes[i], modelPath.string(), assetID, model->m_DefaultMaterials);
                    })
            );
        }

        int nodeCount = CountAssimpNodes(pScene->mRootNode);
        
        // Serializing the nodes
        model->m_Nodes.resize(nodeCount);
        uint32_t index = 0;
        Node* nextNode = model->m_Nodes.data();
        DeserializeNode(nextNode, *pScene->mRootNode, model->m_Meshes, index);

        return model;
    }

    //Ref<Material> ModelLoader::CreatePickMaterial()
    //{
    //    namespace dx = DirectX;

    //    // Single pass
    //    Ref<Material> material = CreateRef<Material>("Pick");

    //    std::string pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PickerPS.cso";
    //    std::string vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PickerVS.cso";

    //    material->m_PixelShader = PixelShader::Create(pixShaderPath);
    //    material->m_VertexShader = VertexShader::Create(vertShaderPath);

    //    DCB::RawLayout layout;
    //    layout.Add(DCB::Type::UInt, "EntityID");

    //    Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));

    //    (*pcb)["EntityID"] = 100;

    //    material->AddBindable(pcb);

    //    return material;
    //}

    Ref<Material> ModelLoader::DeserializeAssimpMaterial(const std::string& basePath, const aiMaterial& aiMat, UUID assetID)
    {
        namespace dx = DirectX;

        // Single pass
        Ref<Material> material = CreateRef<Material>("Opaque", aiMat.GetName().C_Str(), assetID);

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
            material->AddBindable(diffuse);
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
            material->AddBindable(specular);
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
            material->AddBindable(normal);
        }

        // Sampler
        if (material->hasSpecular || material->hasNormalMap || material->hasDiffuseMap)
        {
            material->AddBindable(Sampler::Create(basePath + "mat_" + aiMat.GetName().C_Str()));
        }

        //Blender code if it needs to be added
        /*Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, basePath + "mat_" + std::to_string(index));
        blender->Initialize<Blender>(blending);
        step.AddBindable(blender);
        });//*/

        //Rasterizer
        material->AddBindable(Rasterizer::Create(basePath + "mat_" + aiMat.GetName().C_Str(), hasAlphaDiffuse));

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

        material->m_PixelShader = PixelShader::Create(pixShaderPath);
        material->m_VertexShader = VertexShader::Create(vertShaderPath);

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

            material->AddBindable(pcb);
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

            material->AddBindable(pcb);
        }

        return material;
    }

    Ref<Mesh> ModelLoader::DeserializeMesh(const aiMesh& aiMesh, const std::string& modelPath, UUID assetID, const std::vector<Ref<Material>>& materials)
    {
        namespace dx = DirectX;

        using namespace std::string_literals;

        auto meshTag = modelPath + "%" + aiMesh.mName.C_Str();

        Ref<Mesh> mesh = CreateRef<Mesh>(assetID);

        mesh->m_Name = aiMesh.mName.C_Str();

        mesh->m_DefaultMaterial = materials[aiMesh.mMaterialIndex];

        BufferLayout layout = {
            {"POSITION", ShaderDataType::Float3},
            {"NORMAL", ShaderDataType::Float3},
            {"TANGENT", ShaderDataType::Float3},
            {"BITANGENT", ShaderDataType::Float3},
            {"TEXCOORD", ShaderDataType::Float2}
        };

        VertexShader* vertShader = mesh->m_DefaultMaterial->m_VertexShader.get();
        Ref<VertexBuffer> vertBuffer = VertexBuffer::Create(meshTag, layout, vertShader, aiMesh.mNumVertices);

        {
            PT_PROFILE_SCOPE("Vertex Loading - Model::SerializeMesh");
            for (unsigned int i = 0; i < aiMesh.mNumVertices; i++)
            {
                dx::XMFLOAT3& position = *reinterpret_cast<dx::XMFLOAT3*>(&aiMesh.mVertices[i]);
                dx::XMFLOAT3& normal = *reinterpret_cast<dx::XMFLOAT3*>(&aiMesh.mNormals[i]);
                dx::XMFLOAT3& tangent = *reinterpret_cast<dx::XMFLOAT3*>(&aiMesh.mTangents[i]);
                dx::XMFLOAT3& bitangent = *reinterpret_cast<dx::XMFLOAT3*>(&aiMesh.mBitangents[i]);
                dx::XMFLOAT2& texcoord = *reinterpret_cast<dx::XMFLOAT2*>(&aiMesh.mTextureCoords[0][i]);

                vertBuffer->EmplaceBack(
                    position,
                    normal,
                    tangent,
                    bitangent,
                    texcoord
                );
            }
        }

        mesh->m_VertexBuffer = vertBuffer;

        Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(meshTag, aiMesh.mNumFaces * 3);

        {
            PT_PROFILE_SCOPE("Index Loading - Model::SerializeMesh");
            for (unsigned int i = 0; i < aiMesh.mNumFaces; i++)
            {
                const auto& face = aiMesh.mFaces[i];
                assert(face.mNumIndices == 3);
                indexBuffer->EmplaceBack(face.mIndices[0]);
                indexBuffer->EmplaceBack(face.mIndices[1]);
                indexBuffer->EmplaceBack(face.mIndices[2]);
            }
        }

        mesh->m_IndexBuffer = indexBuffer;
        return mesh;
    }

    Node* ModelLoader::DeserializeNode(Node*& outNode, const aiNode& assimpNode, std::vector<Ref<Mesh>>& meshes, uint32_t& index)
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
            curNode->m_Meshes.push_back(meshes[assimpNode.mMeshes[i]]);
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