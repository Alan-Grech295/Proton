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

			Ref<SharedBindable> texture = CreateRef<SharedBindable>(ResType::Texture2D, basePath + texFileName.C_Str());
			texture->Initialize<Texture2D>(0);
			step.AddBindable(texture);
			hasAlphaDiffuse = static_cast<Texture2D*>(texture.get()->GetRef().get())->HasAlpha();
		}
		else
		{
			aiMat.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
		}

		if (aiMat.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
		{
			matData->hasSpecular = true;
			Ref<SharedBindable> specular = CreateRef<SharedBindable>(ResType::Texture2D, basePath + texFileName.C_Str());
			specular->Initialize<Texture2D>(1);
			//Ref<Texture2D> spec = Texture2D::Create(basePath + texFileName.C_Str(), 1);
			step.AddBindable(specular);
			hasAlphaGloss = dynamic_cast<Texture2D*>(specular.get()->GetRef().get())->HasAlpha();
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
			Ref<SharedBindable> texture = CreateRef<SharedBindable>(ResType::Texture2D, basePath + texFileName.C_Str());
			texture->Initialize<Texture2D>(2);
			step.AddBindable(texture);
			//step.AddBindable(Texture2D::Create(basePath + texFileName.C_Str(), 2));
		}

		if (matData->hasSpecular || matData->hasNormalMap || matData->hasDiffuseMap)
		{
			Ref<SharedBindable> sampler = CreateRef<SharedBindable>(ResType::Sampler, basePath + "mat_" + std::to_string(index));
			sampler->Initialize<Sampler>();
			step.AddBindable(sampler);
			//step.AddBindable(Sampler::Create(meshTag));
		}

		//Blender
		/*Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, basePath + "mat_" + std::to_string(index));
		blender->Initialize<Blender>(blending);
		step.AddBindable(blender);

		//*/
		//Rasterizer
		Ref<SharedBindable> rasterizer = CreateRef<SharedBindable>(ResType::Rasterizer, basePath + "mat_" + std::to_string(index));
		rasterizer->Initialize<Rasterizer>(hasAlphaDiffuse);
		step.AddBindable(rasterizer);
		//

		Ref<SharedBindable> pixShader;
		Ref<SharedBindable> vertShader;

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

		pixShader = CreateRef<SharedBindable>(ResType::PixelShader, pixShaderPath);
		pixShader->Initialize<PixelShader>();
		vertShader = CreateRef<SharedBindable>(ResType::VertexShader, vertShaderPath);
		vertShader->Initialize<VertexShader>();

		step.AddBindable(vertShader);
		step.AddBindable(pixShader);

		if (!matData->hasDiffuseMap && !matData->hasSpecular && !matData->hasNormalMap)
		{
			Ref<UniqueBindable> pcb = CreateRef<UniqueBindable>(ResType::PixelConstantBuffer);

			DCB::RawLayout layout;
			layout.Add(DCB::Type::Float4, "materialColor");
			layout.Add(DCB::Type::Float4, "specularColor");
			layout.Add(DCB::Type::Float, "specularPower");
			pcb->Initialize<PixelConstantBuffer>(1, DCB::CookedLayout(std::move(layout)));
			PixelConstantBuffer& buf = pcb->As<PixelConstantBuffer>();

			buf["materialColor"] = (dx::XMFLOAT4)diffuseColor;
			buf["specularColor"] = (dx::XMFLOAT4)specularColor;
			buf["specularPower"] = (float)shininess;

			step.AddBindable(pcb);
		}
		else
		{
			Ref<UniqueBindable> pcb = CreateRef<UniqueBindable>(ResType::PixelConstantBuffer);

			DCB::RawLayout layout;
			layout.Add(DCB::Type::Float, "specularIntensity");
			layout.Add(DCB::Type::Float, "specularPower");
			layout.Add(DCB::Type::Bool, "hasAlphaGloss");

			pcb->Initialize<PixelConstantBuffer>(1, DCB::CookedLayout(std::move(layout)));
			PixelConstantBuffer& buf = pcb->As<PixelConstantBuffer>();

			buf["specularIntensity"] = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			buf["specularPower"] = shininess;
			buf["hasAlphaGloss"] = hasAlphaGloss;

			step.AddBindable(pcb);
		}

		technique.AddStep(step);

		matData->m_Techniques.push_back(technique);
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
