#include "ptpch.h"
#include "Importer.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Proton
{
	static void SerializeMaterial(Material* matData, const std::string& basePath, uint32_t index, const aiMaterial& aiMat)
	{
		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;

		using namespace std::string_literals;

		std::string matString = std::to_string(index);

		Technique technique = Technique("Opaque");
		Step step = Step("Lambertian");

		float shininess = 40.0f;
		bool hasAlphaGloss = false;
		bool hasAlphaDiffuse = false;
		dx::XMFLOAT4 diffuseColor = { 1.0f,0.0f,1.0f,1.0f };
		dx::XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };

		aiString texFileName;

		if (aiMat.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
		{
			matData->hasDiffuseMap = true;

			Ref<Texture2D> texture = Texture2D::Create(basePath + texFileName.C_Str(), 0);
			step.AddBindable(texture);
			hasAlphaDiffuse = static_cast<Texture2D*>(texture.get())->HasAlpha();
		}
		else
		{
			aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
		}

		if (aiMat.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
		{
			matData->hasSpecular = true;
			Ref<Texture2D> specular = Texture2D::Create(basePath + texFileName.C_Str(), 1);
			//Ref<Texture2D> spec = Texture2D::Create(basePath + texFileName.C_Str(), 1);
			step.AddBindable(specular);
			hasAlphaGloss = dynamic_cast<Texture2D*>(specular.get())->HasAlpha();
		}
		else
		{
			aiMat.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
		}

		if (!hasAlphaGloss)
		{
			aiMat.Get(AI_MATKEY_SHININESS, shininess);
		}

		if (aiMat.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
		{
			matData->hasNormalMap = true;
			Ref<Texture2D> texture = Texture2D::Create(basePath + texFileName.C_Str(), 2);
			step.AddBindable(texture);
			//step.AddBindable(Texture2D::Create(basePath + texFileName.C_Str(), 2));
		}

		if (matData->hasSpecular || matData->hasNormalMap || matData->hasDiffuseMap)
		{
			Ref<Sampler> sampler = Sampler::Create(basePath + "mat_" + std::to_string(index), 0);
			step.AddBindable(sampler);
			//step.AddBindable(Sampler::Create(meshTag));
		}

		//Blender
		/*Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, basePath + "mat_" + std::to_string(index));
		blender->Initialize<Blender>(blending);
		step.AddBindable(blender);

		//*/
		//Rasterizer
		Ref<Rasterizer> rasterizer = Rasterizer::Create(basePath + "mat_" + std::to_string(index), hasAlphaDiffuse);
		step.AddBindable(rasterizer);
		//

		std::string pixShaderPath;
		std::string vertShaderPath;

		if (matData->hasSpecular && !matData->hasNormalMap)
		{
			pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongSpecularPS.cso";
			vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapVS.cso";
		}
		else if (matData->hasNormalMap && !matData->hasSpecular)
		{
			pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapPS.cso";
			vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapVS.cso";
		}
		else if (matData->hasNormalMap && matData->hasSpecular)
		{
			pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapSpecPS.cso";
			vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNormalMapVS.cso";
		}
		else if (!matData->hasNormalMap && !matData->hasSpecular && matData->hasDiffuseMap)
		{
			pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongPS.cso";
			vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongVS.cso";
		}
		else
		{
			pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongNoTexPS.cso";
			vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PhongVS.cso";
		}

		Ref<PixelShader> pixShader = PixelShader::Create(pixShaderPath);
		Ref<VertexShader> vertShader = VertexShader::Create(vertShaderPath);

		step.AddBindable(vertShader);
		step.AddBindable(pixShader);

		if (!matData->hasDiffuseMap && !matData->hasSpecular && !matData->hasNormalMap)
		{
			DCB::RawLayout layout;
			layout.Add(DCB::Type::Float4, "materialColor");
			layout.Add(DCB::Type::Float4, "specularColor");
			layout.Add(DCB::Type::Float, "specularPower");
			Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));
			PixelConstantBuffer& buf = *pcb;

			buf["materialColor"] = (dx::XMFLOAT4)diffuseColor;
			buf["specularColor"] = (dx::XMFLOAT4)specularColor;
			buf["specularPower"] = (float)shininess;

			step.AddBindable(pcb);
		}
		else
		{
			DCB::RawLayout layout;
			layout.Add(DCB::Type::Float, "specularIntensity");
			layout.Add(DCB::Type::Float, "specularPower");
			layout.Add(DCB::Type::Bool, "hasAlphaGloss");

			Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));
			PixelConstantBuffer& buf = *pcb;

			buf["specularIntensity"] = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			buf["specularPower"] = shininess;
			buf["hasAlphaGloss"] = hasAlphaGloss;

			step.AddBindable(pcb);
		}

		technique.AddStep(step);

		matData->m_Techniques.push_back(technique);
	}

	static void SerializeMesh(Mesh* meshData, const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiScene* scene, const Material* materials)
	{
		using ResType = ConstructableBindable::ResourceType;

		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;

		using namespace std::string_literals;

		auto meshTag = modelPath + "%" + mesh.mName.C_Str();

		*meshData = Mesh(mesh.mName.C_Str(), modelPath, materials[mesh.mMaterialIndex]);

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3},
			{"TANGENT", ShaderDataType::Float3},
			{"BITANGENT", ShaderDataType::Float3},
			{"TEXCOORD", ShaderDataType::Float2}
		};

		meshData->m_VertBuffer = CreateRef<SharedBindable>(ResType::VertexBuffer, meshTag);
		VertexShader* vertShader = materials[mesh.mMaterialIndex].m_Techniques[0].m_Steps[0].GetResource<VertexShader>(ResType::VertexShader);
		meshData->m_VertBuffer->Initialize<VertexBuffer>(layout, vertShader, mesh.mNumVertices);

		VertexBuffer* vertBuffer = (VertexBuffer*)(meshData->m_VertBuffer.get()->m_Bindable.get());

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

		meshData->m_IndexBuffer = CreateRef<SharedBindable>(ResType::IndexBuffer, meshTag);
		meshData->m_IndexBuffer->Initialize<IndexBuffer>(mesh.mNumFaces * 3);

		IndexBuffer* indexBuffer = (IndexBuffer*)(meshData->m_IndexBuffer.get()->m_Bindable.get());

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
	}

	Ref<Model> ModelImporter::ImportModel(const std::filesystem::path& path, const ImporterSettings& settings)
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(path.string().c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace);

		Ref<Model> model = CreateRef<Model>();

		if (strlen(imp.GetErrorString()) > 0)
		{
			PT_CORE_ERROR("Error importing model!: {0}", std::string(imp.GetErrorString()));
			return model;
		}

		std::string basePath = std::filesystem::path(path).remove_filename().string();

		aiNode& node = *pScene->mRootNode;

		model->m_Meshes.resize(pScene->mNumMeshes);
		model->m_Materials.resize(pScene->mNumMaterials);

		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			SerializeMaterial(model->m_Materials.data() + i, basePath, i, *pScene->mMaterials[i]);
		}

		for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
		{
			TypeElement meshElement = std::move(SerializeMesh(modelData->m_Meshes.data() + i, rawAsset, basePath, path, *pScene->mMeshes[i], pScene, modelData->m_Materials.data()));
			rawAsset["Meshes"].Add(meshElement);
		}

		model->m_Nodes.resize(CountNodes(pScene->mRootNode));

		uint32_t index = 0;
		Node* nodeData = model->m_Nodes.data();
		SerializeNode(nodeData, pScene->mRootNode, rawAsset, modelData->m_Meshes.data(), index);

		return model;
	}
}
