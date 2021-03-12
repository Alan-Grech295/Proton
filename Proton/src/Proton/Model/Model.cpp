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

namespace Proton
{
	Entity Model::CreateModelEntity(const std::string & path, Scene* activeScene)
	{
		namespace dx = DirectX;

		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(path.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace);

		aiNode& node = *pScene->mRootNode;

		Entity modelEntity = activeScene->CreateEntity(node.mName.C_Str());
		ParentNodeComponent& nodeComponent = modelEntity.AddComponent<ParentNodeComponent>();

		std::string basePath = std::filesystem::path(path).remove_filename().string();

		nodeComponent.meshPtrs.reserve(pScene->mNumMeshes);

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			nodeComponent.meshPtrs.push_back(ParseMesh(basePath, *pScene->mMeshes[i], pScene->mMaterials));
		}

		//Parent Node Creation
		nodeComponent.initialTransform = dx::XMMatrixTranspose(
			dx::XMLoadFloat4x4(
				reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
			)
		);

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(nodeComponent.meshPtrs.at(meshIdx));
		}

		std::vector<Entity> childNodes;

		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			childNodes.push_back(CreateChild(*node.mChildren[i], nodeComponent.meshPtrs, activeScene));
		}

		nodeComponent.childNodes = childNodes;
		nodeComponent.numChildren = childNodes.size();
		MeshComponent& meshComponent = modelEntity.AddComponent<MeshComponent>(curMeshPtrs, curMeshPtrs.size());

		return modelEntity;
	}

	Entity Model::CreateChild(const aiNode& node, std::vector<Mesh*>& meshPtrs, Scene* activeScene)
	{
		//Node Creations
		namespace dx = DirectX;
		const auto transform = dx::XMMatrixTranspose(
			dx::XMLoadFloat4x4(
				reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
			)
		);

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(meshPtrs.at(meshIdx));
		}

		std::vector<Entity> childNodes;

		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			childNodes.push_back(CreateChild(*node.mChildren[i], meshPtrs, activeScene));
		}

		Entity childEntity = activeScene->CreateEntity(node.mName.C_Str());
		ChildNodeComponent& nodeComponent = childEntity.AddComponent<ChildNodeComponent>(transform, childNodes, node.mNumChildren);
		MeshComponent& meshComponent = childEntity.AddComponent<MeshComponent>(curMeshPtrs, curMeshPtrs.size());

		return childEntity;
	}

	Mesh* Model::ParseMesh(const std::string& basePath, const aiMesh& mesh, const aiMaterial* const* pMaterials)
	{
		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
			dx::XMFLOAT3 tangent;
			dx::XMFLOAT3 bitangent;
			dx::XMFLOAT2 uv;
		};

		std::vector<Vertex> vertices;
		vertices.reserve(mesh.mNumVertices);

		{
			PT_PROFILE_SCOPE("Vertex Loading - Model::ParseMesh");
			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				vertices.push_back({
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
					*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
					});
			}
		}

		std::vector<unsigned short> indices;
		indices.reserve((UINT)mesh.mNumFaces * 3);

		{
			PT_PROFILE_SCOPE("Index Loading - Model::ParseMesh");
			for (unsigned int i = 0; i < mesh.mNumFaces; i++)
			{
				const auto& face = mesh.mFaces[i];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}
		}
		
		using namespace std::string_literals;

		auto meshTag = basePath + "%" + mesh.mName.C_Str();

		Mesh* pMesh = new Mesh(meshTag);

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

				pMesh->m_Diffuse = Texture2D::Create(basePath + texFileName.C_Str());		
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasSpecular = true;
				pMesh->m_Specular = Texture2D::Create(basePath + texFileName.C_Str(), 1);
				hasAlphaGloss = pMesh->m_Specular->HasAlpha();
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
				pMesh->m_Normal = Texture2D::Create(basePath + texFileName.C_Str(), 2);
			}

			if(pMesh->hasSpecular || pMesh->hasNormalMap || pMesh->hasDiffuseMap)
				pMesh->m_Sampler = Sampler::Create(meshTag);
		}

		pMesh->m_IndexBuffer = IndexBuffer::Create(meshTag, indices.data(), (uint32_t)indices.size());

		Ref<VertexShader> vs;

		if (pMesh->hasSpecular && !pMesh->hasNormalMap)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongSpecularPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (pMesh->hasNormalMap && !pMesh->hasSpecular)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (pMesh->hasNormalMap && pMesh->hasSpecular)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapSpecPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (!pMesh->hasNormalMap && !pMesh->hasSpecular && pMesh->hasDiffuseMap)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongVS.cso");
		}
		else
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongNoTexPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongVS.cso");
		}
		
		pMesh->m_VertShader = vs;

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3},
			{"TANGENT", ShaderDataType::Float3},
			{"BITANGENT", ShaderDataType::Float3},
			{"TEXCOORD", ShaderDataType::Float2}
		};

		Ref<VertexBuffer> vertBuffer = VertexBuffer::Create(meshTag, sizeof(Vertex), vertices.data(), (uint32_t)vertices.size());

		vertBuffer->SetLayout(layout, &*vs);

		pMesh->m_VertBuffer = vertBuffer;

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
			pMesh->m_MaterialCBuf = PixelConstantBuffer::CreateUnique(1, sizeof(pmc), &pmc);
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
			pMesh->m_MaterialCBuf = PixelConstantBuffer::CreateUnique(1, sizeof(pmc), &pmc);
		}

		return pMesh;
	}

	void Mesh::Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform, DirectX::FXMMATRIX cameraView, DirectX::FXMMATRIX projectionMatrix) const
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

		callback(m_IndexBuffer->GetCount());
	}
}