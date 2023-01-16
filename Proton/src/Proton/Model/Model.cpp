#include "ptpch.h"
#include "Model.h"
#include "Proton\Renderer\Renderer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Proton\Core\Log.h"
#include <unordered_map>
#include <imgui.h>
#include <filesystem>

#include "Proton\Scene\Scene.h"
#include "Proton\Scene\Entity.h"
#include "Proton\Scene\Components.h"

#include "Proton\Asset Loader\AssetManager.h"
#include "Proton\Scene\ModelCollection.h"

namespace Proton
{
	/*Entity ModelCreator::CreateModelEntity(const std::string& path, Scene* activeScene)
	{
		namespace dx = DirectX;

		Ref<Model> model = AssetManager::GetModel(path);

		Node* node = model->rootNode;

		Entity modelEntity = activeScene->CreateEntity(node->name);
		NodeComponent& nodeComponent = modelEntity.GetComponent<NodeComponent>();
		nodeComponent.m_PrefabName = "";

		PT_CORE_TRACE((uint32_t)&nodeComponent);

		std::string basePath = std::filesystem::path(path).remove_filename().string();

		//Parent Node Creation
		nodeComponent.m_NodeName = node->name;
		nodeComponent.m_Origin = node->transformation;

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node->numMeshes);
		
		for (size_t i = 0; i < node->numMeshes; i++)
		{
			curMeshPtrs.push_back(node->meshes[i]);
		}

		for (size_t i = 0; i < node->numChildren; i++)
		{
			CreateChild(*node->childNodes[i], modelEntity, modelEntity, activeScene).SetParent(&modelEntity);
		}

		MeshComponent& meshComponent = modelEntity.AddComponent<MeshComponent>(curMeshPtrs, curMeshPtrs.size());

		for (auto child : nodeComponent.m_ChildNodes)
		{
			PT_CORE_TRACE(child.GetComponent<TagComponent>().tag);
		}

		return modelEntity;
	}

	Entity ModelCreator::CreatePrefabEntity(const std::string& path, Scene* activeScene)
	{
		namespace dx = DirectX;

		Ref<Prefab> prefab = AssetManager::GetPrefab(path);

		Entity modelEntity = activeScene->CreateEntity(prefab->rootNode->name);
		TransformComponent& transformComponent = modelEntity.GetComponent<TransformComponent>();
		NodeComponent& nodeComponent = modelEntity.GetComponent<NodeComponent>();
		nodeComponent.m_PrefabName = path;

		std::string basePath = std::filesystem::path(path).remove_filename().string();

		PrefabNode* node = prefab->rootNode;

		//Parent Node Creation
		nodeComponent.m_NodeName = node->name;
		nodeComponent.m_Origin = node->transformation;
		transformComponent.position = node->position;
		transformComponent.rotation = node->rotation;
		transformComponent.scale = node->scale;

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node->numMeshes);

		for (size_t i = 0; i < node->numMeshes; i++)
		{
			curMeshPtrs.push_back(node->meshes[i]);
		}

		for (size_t i = 0; i < node->numChildren; i++)
		{
			CreatePrefabChild(*node->childNodes[i], modelEntity, modelEntity, activeScene).SetParent(&modelEntity);
		}

		MeshComponent& meshComponent = modelEntity.AddComponent<MeshComponent>(curMeshPtrs, curMeshPtrs.size());

		return modelEntity;
	}

	Entity ModelCreator::CreateChild(const Node& node, Entity parent, Entity root, Scene* activeScene)
	{
		//Node Creations
		namespace dx = DirectX;
		const auto transform = node.transformation;

		Entity childEntity = activeScene->CreateEntity(node.name);

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.numMeshes);
		for (size_t i = 0; i < node.numMeshes; i++)
		{
			curMeshPtrs.push_back(node.meshes[i]);
		}

		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.m_NodeName = node.name;
		nodeComponent.m_Origin = transform;
		//nodeComponent.m_ParentEntity = parent;
		//nodeComponent.m_RootEntity = root;
		nodeComponent.m_PrefabName = "";

		for (size_t i = 0; i < node.numChildren; i++)
		{
			CreateChild(*node.childNodes[i], childEntity, root, activeScene).SetParent(&childEntity);
		}

		MeshComponent& meshComponent = childEntity.AddComponent<MeshComponent>(curMeshPtrs, curMeshPtrs.size());

		return childEntity;
	}

	Entity ModelCreator::CreatePrefabChild(const PrefabNode& node, Entity parent, Entity root, Scene* activeScene)
	{
		//Node Creations
		namespace dx = DirectX;
		const auto transform = node.transformation;

		Entity childEntity = activeScene->CreateEntity(node.name);
		TransformComponent& transformComponent = childEntity.GetComponent<TransformComponent>();
		transformComponent.position = node.position;
		transformComponent.rotation = node.rotation;
		transformComponent.scale = node.scale;

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.numMeshes);
		for (size_t i = 0; i < node.numMeshes; i++)
		{
			curMeshPtrs.push_back(node.meshes[i]);
		}

		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.m_NodeName = node.name;
		nodeComponent.m_Origin = transform;
		nodeComponent.m_PrefabName = "";

		for (size_t i = 0; i < node.numChildren; i++)
		{
			CreatePrefabChild(*node.childNodes[i], childEntity, root, activeScene).SetParent(&childEntity);
		}

		MeshComponent& meshComponent = childEntity.AddComponent<MeshComponent>(curMeshPtrs, curMeshPtrs.size());

		return childEntity;
	}

	Mesh* ModelCreator::ParseMesh(const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiMaterial* const* pMaterials)
	{
		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;

		using namespace std::string_literals;

		auto meshTag = modelPath + "%" + mesh.mName.C_Str();

		Mesh* pMesh = new Mesh(meshTag, mesh.mName.C_Str(), modelPath);

		ModelCollection::AddMesh(meshTag, pMesh);

		/*struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
			dx::XMFLOAT3 tangent;
			dx::XMFLOAT3 bitangent;
			dx::XMFLOAT2 uv;
		};//*//*

		//Technique creation
		Technique technique = Technique("Opaque");
		Step step = Step("Lambertian");

		step.AddBindable(pMesh->m_TransformCBuf);
		step.AddBindable(pMesh->m_TransformCBufPix);

		float shininess = 40.0f;
		bool hasAlphaGloss = false;
		dx::XMFLOAT4 diffuseColor = { 1.0f,0.0f,1.0f,1.0f };
		dx::XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };

		if (mesh.mMaterialIndex >= 0)
		{
			auto& material = *pMaterials[mesh.mMaterialIndex];

			aiString texFileName;
			
			if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasDiffuseMap = true;
				PT_CORE_TRACE(texFileName.C_Str());

				step.AddBindable(Texture2D::Create(basePath + texFileName.C_Str()));
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasSpecular = true;
				Ref<Texture2D> spec = Texture2D::Create(basePath + texFileName.C_Str(), 1);
				step.AddBindable(spec);
				hasAlphaGloss = spec->HasAlpha();
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
			}

			if (!hasAlphaGloss)
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasNormalMap = true;
				step.AddBindable(Texture2D::Create(basePath + texFileName.C_Str(), 2));
			}

			if (pMesh->hasSpecular || pMesh->hasNormalMap || pMesh->hasDiffuseMap)
				step.AddBindable(Sampler::Create(meshTag));
		}

		Ref<PixelShader> pixShader;
		Ref<VertexShader> vertShader;

		if (pMesh->hasSpecular && !pMesh->hasNormalMap)
		{
			pixShader = PixelShader::Create("D:\\Dev\\Proton\\Proton\\PhongSpecularPS.cso");
			vertShader = VertexShader::Create("D:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (pMesh->hasNormalMap && !pMesh->hasSpecular)
		{
			pixShader = PixelShader::Create("D:\\Dev\\Proton\\Proton\\PhongNormalMapPS.cso");
			vertShader = VertexShader::Create("D:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (pMesh->hasNormalMap && pMesh->hasSpecular)
		{
			pixShader = PixelShader::Create("D:\\Dev\\Proton\\Proton\\PhongNormalMapSpecPS.cso");
			vertShader = VertexShader::Create("D:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (!pMesh->hasNormalMap && !pMesh->hasSpecular && pMesh->hasDiffuseMap)
		{
			pixShader = PixelShader::Create("D:\\Dev\\Proton\\Proton\\PhongPS.cso");
			vertShader = VertexShader::Create("D:\\Dev\\Proton\\Proton\\PhongVS.cso");
		}
		else
		{
			pixShader = PixelShader::Create("D:\\Dev\\Proton\\Proton\\PhongNoTexPS.cso");
			vertShader = VertexShader::Create("D:\\Dev\\Proton\\Proton\\PhongVS.cso");
		}

		step.AddBindable(vertShader);
		step.AddBindable(pixShader);

		//std::vector<Vertex> vertices;
		//vertices.reserve(mesh.mNumVertices);

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3},
			{"TANGENT", ShaderDataType::Float3},
			{"BITANGENT", ShaderDataType::Float3},
			{"TEXCOORD", ShaderDataType::Float2}
		};

		pMesh->m_VertBuffer = VertexBuffer::Create(meshTag, layout, vertShader.get());

		{
			PT_PROFILE_SCOPE("Vertex Loading - Model::ParseMesh");
			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				pMesh->m_VertBuffer->EmplaceBack(
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
					*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
					);
			}
		}

		pMesh->m_IndexBuffer = IndexBuffer::Create(meshTag);
		//indices.reserve((UINT)mesh.mNumFaces * 3);
		{
			PT_PROFILE_SCOPE("Index Loading - Model::ParseMesh");
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				pMesh->m_IndexBuffer->EmplaceBack(face.mIndices[0]);
				pMesh->m_IndexBuffer->EmplaceBack(face.mIndices[1]);
				pMesh->m_IndexBuffer->EmplaceBack(face.mIndices[2]);
			}
		}

		if (!pMesh->hasDiffuseMap && !pMesh->hasSpecular && !pMesh->hasNormalMap)
		{
			struct PSMaterialConstantNoTex
			{
				dx::XMFLOAT4 materialColor;
				dx::XMFLOAT4 specularColor;
				float specularPower;
				float padding[3];
			} pmc;

			pmc.specularPower = shininess;
			pmc.specularColor = specularColor;
			pmc.materialColor = diffuseColor;
			step.AddBindable(PixelConstantBuffer::CreateUnique(1, sizeof(pmc), &pmc));
		}
		else
		{
			struct PSMaterialConstant
			{
				float specularIntensity;
				float specularPower;
				BOOL hasAlphaGloss;
				float padding;
			} pmc;

			pmc.specularPower = shininess;
			pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			pmc.hasAlphaGloss = hasAlphaGloss ? TRUE : FALSE;
			step.AddBindable(PixelConstantBuffer::CreateUnique(1, sizeof(pmc), &pmc));
		}

		technique.AddStep(step);

		pMesh->m_Techniques.push_back(std::move(technique));

		return pMesh;
	}*/

	Element ModelCreator::ParseMesh(const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiScene* scene)
	{
		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;

		using namespace std::string_literals;

		Element retElement = Element(modelPath.c_str(), Type::Struct);

		/*if (mesh.mMaterialIndex >= 0)
		{
			auto& material = *scene->mMaterials[mesh.mMaterialIndex];

			aiString texFileName;

			if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasDiffuseMap = true;
				PT_CORE_TRACE(texFileName.C_Str());

				step.AddBindable(Texture2D::Create(basePath + texFileName.C_Str()));
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasSpecular = true;
				Ref<Texture2D> spec = Texture2D::Create(basePath + texFileName.C_Str(), 1);
				step.AddBindable(spec);
				hasAlphaGloss = spec->HasAlpha();
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
			}

			if (!hasAlphaGloss)
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasNormalMap = true;
				step.AddBindable(Texture2D::Create(basePath + texFileName.C_Str(), 2));
			}

			if (pMesh->hasSpecular || pMesh->hasNormalMap || pMesh->hasDiffuseMap)
				step.AddBindable(Sampler::Create(meshTag));
		}*/

		return retElement;
	}

	/*void Mesh::Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform, DirectX::FXMMATRIX cameraView, DirectX::FXMMATRIX projectionMatrix) const
	{
		const auto modelView = accumulatedTransform * cameraView;
		const Transforms tf =
		{
			DirectX::XMMatrixTranspose(modelView),
			DirectX::XMMatrixTranspose(
				modelView *
				projectionMatrix
			)
		};

		m_IndexBuffer->Bind();
		m_VertBuffer->Bind();
		m_VertShader->Bind();
		m_PixelShader->Bind();

		m_TransformCBuf->SetData(sizeof(Transforms), &tf);
		m_TransformCBuf->Bind();
		m_MaterialCBuf->Bind();

		if(hasDiffuseMap)
			m_Diffuse->Bind();

		if(hasSpecular)
			m_Specular->Bind();

		if (hasNormalMap)
		{
			m_Normal->Bind();
			m_TransformCBufPix->SetData(sizeof(Transforms), &tf);
			m_TransformCBufPix->Bind();
		}

		if (hasNormalMap || hasSpecular || hasDiffuseMap)
		{
			m_Sampler->Bind();
		}

		callback(m_IndexBuffer->size());
	}*/
}