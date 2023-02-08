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

namespace Proton
{
	//std::string ModelCreator::MAT_TAG = "Material_";

	Entity ModelCreator::CreateModelEntity(const std::string& path, Scene& activeScene)
	{
		namespace dx = DirectX;

		ModelData& modelData = AssetCollection::Get<ModelData>(path);

		NodeData& rootNode = modelData.m_Nodes[0];

		Ref<Model> model = CreateRef<Model>();

		model->m_Materials.reserve(modelData.m_Materials.size());
		model->m_Meshes.reserve(modelData.m_Meshes.size());

		for (MaterialData& matData : modelData.m_Materials)
		{
			model->m_Materials.push_back(matData.CreateMaterial());
		}

		for (MeshData& meshData : modelData.m_Meshes)
		{
			model->m_Meshes.push_back(meshData.CreateMesh(model->m_Materials));
		}

		return CreateNode(rootNode, modelData.m_Nodes.data(), model->m_Meshes.data(), model, activeScene);
	}

	Entity ModelCreator::CreateNode(NodeData& nodeData, NodeData* nodes, Mesh* meshes, Ref<Model> modelRef, Scene& activeScene)
	{
		//Node Creations
		namespace dx = DirectX;
		Entity childEntity = activeScene.CreateEntity(nodeData.m_Name);
		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.m_NodeName = nodeData.m_Name;
		nodeComponent.m_Origin = nodeData.m_Transformation;

		nodeComponent.m_Children.reserve(nodeData.m_Children.size());

		MeshComponent& meshComponent = childEntity.AddComponent<MeshComponent>();
		meshComponent.m_ModelRef = modelRef;
		meshComponent.m_MeshPtrs.reserve(nodeData.m_Meshes.size());

		for (uint32_t meshIndex : nodeData.m_Meshes)
		{
			meshComponent.m_MeshPtrs.push_back(meshes + meshIndex);
		}

		for (uint32_t childIndex : nodeData.m_Children)
		{
			CreateNode(nodes[childIndex], nodes, meshes, modelRef, activeScene).SetParent(&childEntity);
		}

		return childEntity;
	}

	TypeElement ModelCreator::SerializeMesh(MeshData* meshData, RawAsset& asset, const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiScene* scene, const MaterialData* materials)
	{
		using ResType = ConstructableBindable::ResourceType;

		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;

		using namespace std::string_literals;

		auto meshTag = modelPath + "%" + mesh.mName.C_Str();

		meshData->Create(mesh.mName.C_Str(), modelPath);
		meshData->m_Material = mesh.mMaterialIndex;

		TypeElement meshElement = TypeElement({
			Element::Create("MaterialIndex", const_cast<uint32_t&>(mesh.mMaterialIndex)),
			Element::Create("Topology", (uint32_t)static_cast<Topology*>(meshData->m_Topology->GetRef().get())->GetTopology()),
			Element("Name", Type::Pointer),
			Element("Vertices", Type::Pointer),
			Element("Indices", Type::Pointer),
			});

		meshElement["Name"].SetPointer(Type::String, asset);
		meshElement["Name"]->SetData(meshData->m_Name);

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

		meshElement["Vertices"].SetPointer(Type::Array, asset);
		TypeElement& vertArray = meshElement["Vertices"]->SetSize(mesh.mNumVertices).SetType(TypeElement({
			Element("POSITION", {
				Element("X", Type::Float),
				Element("Y", Type::Float),
				Element("Z", Type::Float),
			}),
			Element("NORMAL", {
				Element("X", Type::Float),
				Element("Y", Type::Float),
				Element("Z", Type::Float)
			}),
			Element("TANGENT", {
				Element("X", Type::Float),
				Element("Y", Type::Float),
				Element("Z", Type::Float)
			}),
			Element("BITANGENT", {
				Element("X", Type::Float),
				Element("Y", Type::Float),
				Element("Z", Type::Float)
			}),
			Element("TEXCOORD", {
				Element("X", Type::Float),
				Element("Y", Type::Float)
			})
		}));

		{
			PT_PROFILE_SCOPE("Vertex Loading - Model::SerializeMesh");
			for (unsigned int i = 0; i < mesh.mNumVertices; i++)
			{
				dx::XMFLOAT3& position	= *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]);
				dx::XMFLOAT3& normal	= *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]);
				dx::XMFLOAT3& tangent	= *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]);
				dx::XMFLOAT3& bitangent	= *reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]);
				dx::XMFLOAT2& texcoord	= *reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i]);

				vertBuffer->EmplaceBack(
					position,
					normal,
					tangent,
					bitangent,
					texcoord
				);

				vertArray.Add(TypeElement({
					Element("POSITION", {
						Element::Create("X", position.x),
						Element::Create("Y", position.y),
						Element::Create("Z", position.z),
					}),
					Element("NORMAL", {
						Element::Create("X", normal.x),
						Element::Create("Y", normal.y),
						Element::Create("Z", normal.z),
					}),
					Element("TANGENT", {
						Element::Create("X", tangent.x),
						Element::Create("Y", tangent.y),
						Element::Create("Z", tangent.z),
					}),
					Element("BITANGENT", {
						Element::Create("X", bitangent.x),
						Element::Create("Y", bitangent.y),
						Element::Create("Z", bitangent.z),
					}),
					Element("TEXCOORD", {
						Element::Create("X", texcoord.x),
						Element::Create("Y", texcoord.y),
					})
					}));
			}
		}

		meshData->m_IndexBuffer = CreateRef<SharedBindable>(ResType::IndexBuffer, meshTag);
		meshData->m_IndexBuffer->Initialize<IndexBuffer>(mesh.mNumFaces * 3);

		meshElement["Indices"].SetPointer(Type::Array, asset);
		TypeElement& indArray = meshElement["Indices"]->SetSize(mesh.mNumFaces * 3).SetType(Type::UInt32);

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

				indArray.Add({
					TypeElement::Create(face.mIndices[0]),
					TypeElement::Create(face.mIndices[1]),
					TypeElement::Create(face.mIndices[2])
				});
			}
		}



		return meshElement;
	}

	ModelData ModelCreator::Serialize(const std::string& path)
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(path.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace);

		ModelData modelData = ModelData();

		if (strlen(imp.GetErrorString()) > 0)
		{
			PT_CORE_ERROR("Error importing model!: {0}", std::string(imp.GetErrorString()));
			return modelData;
		}

		std::string basePath = std::filesystem::path(path).remove_filename().string();

		aiNode& node = *pScene->mRootNode;

		modelData.m_Meshes.resize(pScene->mNumMeshes);
		modelData.m_Materials.resize(pScene->mNumMaterials);

		RawAsset rawAsset = RawAsset();

		//TODO: Move to Asset Manager
		rawAsset.Add("AssetType", (uint32_t)AssetType::Model);

		rawAsset.Add("Materials", Type::Struct);

		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			SerializeMaterial(modelData.m_Materials.data() + i, rawAsset, basePath, i, *pScene->mMaterials[i]);
		}

		rawAsset.Add("Meshes", Type::Array);
		rawAsset["Meshes"].SetSize(pScene->mNumMeshes).SetType(TypeElement({
			Element("MaterialIndex", Type::UInt32),
			Element("Topology", Type::UInt32),
			Element("Name", Type::Pointer),
			Element("Vertices", Type::Pointer),
			Element("Indices", Type::Pointer),
		}));

		for (uint32_t i = 0; i < pScene->mNumMeshes; i++)
		{
			TypeElement meshElement = std::move(SerializeMesh(modelData.m_Meshes.data() + i, rawAsset, basePath, path, *pScene->mMeshes[i], pScene, modelData.m_Materials.data()));
			rawAsset["Meshes"].Add(meshElement);
		}

		modelData.m_Nodes.resize(CountNodes(pScene->mRootNode));
		rawAsset.Add("Nodes", Type::Struct);

		uint32_t index = 0;
		NodeData* nodeData = modelData.m_Nodes.data();
		SerializeNode(nodeData, pScene->mRootNode, rawAsset, modelData.m_Meshes.data(), index);

		Asset asset = Asset(rawAsset);

		AssetSerializer::SerializeAsset(path + ".asset", asset);

		return modelData;
	}

	void ModelCreator::SerializeMaterial(MaterialData* matData, RawAsset& asset, const std::string& basePath, uint32_t index, const aiMaterial& aiMat)
	{
		using ResType = ConstructableBindable::ResourceType;

		PT_PROFILE_FUNCTION();

		namespace dx = DirectX;

		using namespace std::string_literals;

		std::string matString = std::to_string(index);
		asset["Materials"].Add(matString.c_str(), Type::Struct);
		asset["Materials"][matString].Add("Techniques", Type::Struct);
		asset["Materials"][matString]["Techniques"].Add("Opaque", Type::Array);

		asset["Materials"][matString]["Techniques"]["Opaque"].SetType(Type::Struct);
		asset["Materials"][matString]["Techniques"]["Opaque"].GetType().Add({
			Element("StepID", Type::UInt16),
			Element("ResType", Type::Int32),
			Element("Shared", Type::Bool),
			Element("Data", Type::Pointer)
		});

		TechniqueData technique = TechniqueData("Opaque");
		StepData step = StepData("Lambertian");

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

			//Asset Data
			uint32_t itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
				Element::Create("StepID", (uint16_t)step.m_ID),
				Element::Create("ResType", (int)ResType::Texture2D),
				Element::Create("Shared", true),
				Element("Data", Type::Pointer)
			}));

			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
				Element::Create("Path", basePath + texFileName.C_Str()),
				Element::Create("Slot", 0)
				});
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

			//Asset Data
			uint32_t itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
				Element::Create("StepID", (uint16_t)step.m_ID),
				Element::Create("ResType", (int)ResType::Texture2D),
				Element::Create("Shared", true),
				Element("Data", Type::Pointer)
				}));

			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
				Element::Create("Path", basePath + texFileName.C_Str()),
				Element::Create("Slot", 1)
			});
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

			//Asset Data
			uint32_t itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
				Element::Create("StepID", (uint16_t)step.m_ID),
				Element::Create("ResType", (int)ResType::Texture2D),
				Element::Create("Shared", true),
				Element("Data", Type::Pointer)
				}));

			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
				Element::Create("Path", basePath + texFileName.C_Str()),
				Element::Create("Slot", 2)
				});
		}

		if (matData->hasSpecular || matData->hasNormalMap || matData->hasDiffuseMap)
		{
			Ref<SharedBindable> sampler = CreateRef<SharedBindable>(ResType::Sampler, basePath + "mat_" + std::to_string(index));
			sampler->Initialize<Sampler>();
			step.AddBindable(sampler);
			//step.AddBindable(Sampler::Create(meshTag));

			//Asset Data
			uint32_t itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
				Element::Create("StepID", (uint16_t)step.m_ID),
				Element::Create("ResType", (int)ResType::Sampler),
				Element::Create("Shared", true),
				Element("Data", Type::Pointer)
			}));

			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::String, asset);
			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->SetData(std::string(basePath + "mat_" + std::to_string(index)));
		}

		//Blender
		/*Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, basePath + "mat_" + std::to_string(index));
		blender->Initialize<Blender>(blending);
		step.AddBindable(blender);

		//Asset Data
		uint32_t itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
			Element::Create("StepID", (uint16_t)step.m_ID),
			Element::Create("ResType", (int)ResType::Blender),
			Element::Create("Shared", true),
			Element("Data", Type::Pointer)
			}));

		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
			Element::Create("Tag", std::string(basePath + "mat_" + std::to_string(index))),
			Element::Create("Blending", blending)
		});*/

		//
		//Rasterizer
		Ref<SharedBindable> rasterizer = CreateRef<SharedBindable>(ResType::Rasterizer, basePath + "mat_" + std::to_string(index));
		rasterizer->Initialize<Rasterizer>(hasAlphaDiffuse);
		step.AddBindable(rasterizer);

		//Asset Data
		uint32_t itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
			Element::Create("StepID", (uint16_t)step.m_ID),
			Element::Create("ResType", (int)ResType::Rasterizer),
			Element::Create("Shared", true),
			Element("Data", Type::Pointer)
			}));

		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
			Element::Create("Tag", std::string(basePath + "mat_" + std::to_string(index))),
			Element::Create("TwoSided", hasAlphaDiffuse)
		});
		//

		asset["Materials"][matString].Add({
			Element::Create("HasDiffuse", matData->hasDiffuseMap),
			Element::Create("HasSpecular", matData->hasSpecular),
			Element::Create("HasNormal", matData->hasNormalMap)
		});

		Ref<SharedBindable> pixShader;
		Ref<SharedBindable> vertShader;

		std::string pixShaderPath;
		std::string vertShaderPath;

		if (matData->hasSpecular && !matData->hasNormalMap)
		{
			pixShaderPath = "D:\\Dev\\Proton\\Proton\\PhongSpecularPS.cso";
			vertShaderPath = "D:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso";
		}
		else if (matData->hasNormalMap && !matData->hasSpecular)
		{
			pixShaderPath = "D:\\Dev\\Proton\\Proton\\PhongNormalMapPS.cso";
			vertShaderPath = "D:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso";
		}
		else if (matData->hasNormalMap && matData->hasSpecular)
		{
			pixShaderPath = "D:\\Dev\\Proton\\Proton\\PhongNormalMapSpecPS.cso";
			vertShaderPath = "D:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso";
		}
		else if (!matData->hasNormalMap && !matData->hasSpecular && matData->hasDiffuseMap)
		{
			pixShaderPath = "D:\\Dev\\Proton\\Proton\\PhongPS.cso";
			vertShaderPath = "D:\\Dev\\Proton\\Proton\\PhongVS.cso";
		}
		else
		{
			pixShaderPath = "D:\\Dev\\Proton\\Proton\\PhongNoTexPS.cso";
			vertShaderPath = "D:\\Dev\\Proton\\Proton\\PhongVS.cso";
		}

		pixShader = CreateRef<SharedBindable>(ResType::PixelShader, pixShaderPath);
		pixShader->Initialize<PixelShader>();
		vertShader = CreateRef<SharedBindable>(ResType::VertexShader, vertShaderPath);
		vertShader->Initialize<VertexShader>();

		step.AddBindable(vertShader);
		step.AddBindable(pixShader);

		//Asset Data
		itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
			Element::Create("StepID", (uint16_t)step.m_ID),
			Element::Create("ResType", (int)ResType::PixelShader),
			Element::Create("Shared", true),
			Element("Data", Type::Pointer)
			}));

		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::String, asset);
		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->SetData(pixShaderPath);

		//Asset Data
		itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
			Element::Create("StepID", (uint16_t)step.m_ID),
			Element::Create("ResType", (int)ResType::VertexShader),
			Element::Create("Shared", true),
			Element("Data", Type::Pointer)
			}));

		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::String, asset);
		asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->SetData(vertShaderPath);

		if (!matData->hasDiffuseMap && !matData->hasSpecular && !matData->hasNormalMap)
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

			Ref<UniqueBindable> pcb = CreateRef<UniqueBindable>(ResType::PixelConstantBuffer);
			pcb->Initialize<PixelConstantBuffer>(1, sizeof(pmc), &pmc);
			step.AddBindable(pcb);
			
			//Asset Data
			itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
				Element::Create("StepID", (uint16_t)step.m_ID),
				Element::Create("ResType", (int)ResType::PixelConstantBuffer),
				Element::Create("Shared", false),
				Element("Data", Type::Pointer)
				}));

			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
				Element::Create("Slot", 1),
				Element("MaterialCol", {
					Element::Create("X", pmc.materialColor.x),
					Element::Create("Y", pmc.materialColor.y),
					Element::Create("Z", pmc.materialColor.z),
					Element::Create("W", pmc.materialColor.w),
				}),
				Element("SpecCol", {
					Element::Create("X", pmc.specularColor.x),
					Element::Create("Y", pmc.specularColor.y),
					Element::Create("Z", pmc.specularColor.z),
					Element::Create("W", pmc.specularColor.w),
				}),
				Element::Create("SpecPow", pmc.specularPower)
			});
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

			Ref<UniqueBindable> pcb = CreateRef<UniqueBindable>(ResType::PixelConstantBuffer);
			pcb->Initialize<PixelConstantBuffer>(1, sizeof(pmc), &pmc);
			step.AddBindable(pcb);

			//Asset Data
			itemIndex = asset["Materials"][matString]["Techniques"]["Opaque"].Add(TypeElement({
				Element::Create("StepID", (uint16_t)step.m_ID),
				Element::Create("ResType", (int)ResType::PixelConstantBuffer),
				Element::Create("Shared", false),
				Element("Data", Type::Pointer)
				}));

			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"].SetPointer(Type::Struct, asset);
			asset["Materials"][matString]["Techniques"]["Opaque"][itemIndex]["Data"]->Add({
				Element::Create("Slot", 1),
				Element::Create("SpecIntensity", pmc.specularIntensity),
				Element::Create("SpecPower", pmc.specularPower),
				Element::Create("HasAlphaGloss", pmc.hasAlphaGloss),
			});
		}

		technique.m_Steps.push_back(step);

		matData->m_Techniques.push_back(technique);
	}

	void ModelCreator::SerializeNode(NodeData*& nodeData, aiNode* aiNode, RawAsset& asset, MeshData* meshes, uint32_t& index)
	{
		assert("Node Data at position already initialized" && (nodeData->m_Name == ""));
		nodeData->m_Name = aiNode->mName.C_Str();
		asset["Nodes"].Add(nodeData->m_Name.c_str(), Type::Struct);
		nodeData->m_Transformation = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(
				reinterpret_cast<const DirectX::XMFLOAT4X4*>(&aiNode->mTransformation)
			)
		);

		asset["Nodes"][nodeData->m_Name].Add({ 
			Element::Create("Index", index),
			Element("TransformationMat", Type::Array)
		});

		Element& transformElement = asset["Nodes"][nodeData->m_Name]["TransformationMat"].SetSize(16).SetType(TypeElement(Type::Float));
		float* nextMatFloat = (float*)&nodeData->m_Transformation;
		for (int i = 0; i < 16; i++)
		{
			transformElement.Add(*nextMatFloat);
			nextMatFloat++;
		}

		if (aiNode->mNumMeshes > 0)
		{
			Element& meshesElement = asset["Nodes"][nodeData->m_Name].Add("Meshes", Type::Array);
			meshesElement.SetSize(aiNode->mNumMeshes).SetType(Type::UInt32);

			nodeData->m_Meshes.reserve(aiNode->mNumMeshes);
			memcpy(nodeData->m_Meshes.data(), aiNode->mMeshes, aiNode->mNumMeshes * sizeof(unsigned int));

			for (uint32_t i = 0; i < aiNode->mNumMeshes; i++)
			{
				nodeData->m_Meshes.push_back(aiNode->mMeshes[i]);
				meshesElement.Add((uint32_t)aiNode->mMeshes[i]);
			}
		}

		if (aiNode->mNumChildren > 0)
		{
			asset["Nodes"][nodeData->m_Name].Add("ChildNodes", Type::Array);
			asset["Nodes"][nodeData->m_Name]["ChildNodes"].SetSize(aiNode->mNumChildren).SetType(Type::UInt32);

			nodeData->m_Children.reserve(aiNode->mNumChildren);

			NodeData* curNode = nodeData;

			for (uint32_t i = 0; i < aiNode->mNumChildren; i++)
			{
				index++;
				nodeData++;

				curNode->m_Children.push_back(index);

				asset["Nodes"][curNode->m_Name]["ChildNodes"].Add(TypeElement::Create(index));
				SerializeNode(nodeData, aiNode->mChildren[i], asset, meshes, index);
			}
		}
	}
	
	uint32_t ModelCreator::CountNodes(aiNode* root)
	{
		if (root->mNumChildren == 0)
			return 1;

		uint32_t sum = 1;
		for (uint32_t i = 0; i < root->mNumChildren; i++)
		{
			sum += CountNodes(root->mChildren[i]);
		}

		return sum;
	}

	ModelData ModelCreator::Deserialize(Asset& modelAsset, const std::string& path)
	{
		ModelData modelData = ModelData();
		modelData.m_Materials.resize(modelAsset["Materials"].Size());
		modelData.m_Meshes.resize(modelAsset["Meshes"].Size());
		modelData.m_Nodes.resize(modelAsset["Nodes"].Size());

		DeserializeMaterials(modelAsset, modelData.m_Materials.data());

		DeserializeMeshes(modelAsset, path, modelData.m_Meshes.data(), modelData.m_Materials.data());

		DeserializeNodes(modelAsset, modelData.m_Nodes.data(), modelData.m_Meshes.data());

		return modelData;
	}

	void ModelCreator::DeserializeMeshes(Asset& asset, const std::string& modelPath, MeshData* meshData, const MaterialData* materials)
	{
		using ResType = ConstructableBindable::ResourceType;
		for (uint32_t i = 0; i < asset["Meshes"].Size(); i++, meshData++)
		{
			ElementRef& mesh = asset["Meshes"][i];
			std::string meshTag = modelPath + "%" + (std::string)*mesh["Name"];
			meshData->Create(*mesh["Name"], modelPath);
			meshData->m_Material = (uint32_t)mesh["MaterialIndex"];
			//Set topology?
			//meshData->m_Topology

			ElementRef vertices = *(mesh["Vertices"]);
			ElementRef indices = *(mesh["Indices"]);

			BufferLayout layout = {
				{"POSITION", ShaderDataType::Float3},
				{"NORMAL", ShaderDataType::Float3},
				{"TANGENT", ShaderDataType::Float3},
				{"BITANGENT", ShaderDataType::Float3},
				{"TEXCOORD", ShaderDataType::Float2}
			};

			//Vertices
			meshData->m_VertBuffer = CreateRef<SharedBindable>(ResType::VertexBuffer, meshTag);
			VertexShader* vertShader = materials[meshData->m_Material].m_Techniques[0].m_Steps[0].GetResource<VertexShader>(ResType::VertexShader);
			meshData->m_VertBuffer->Initialize<VertexBuffer>(layout, vertShader, vertices.Size());

			VertexBuffer* vertBuffer = (VertexBuffer*)meshData->m_VertBuffer.get()->GetRef().get();

			vertBuffer->SetRawData(vertices.m_Data, vertices.m_Size);

			//Indices
			meshData->m_IndexBuffer = CreateRef<SharedBindable>(ResType::IndexBuffer, meshTag);
			meshData->m_IndexBuffer->Initialize<IndexBuffer>(indices.Size());

			IndexBuffer* indexBuffer = (IndexBuffer*)meshData->m_IndexBuffer.get()->GetRef().get();

			indexBuffer->SetRawData((uint32_t*)indices.m_Data, indices.Size());
		}
	}

	void ModelCreator::DeserializeMaterials(Asset& asset, MaterialData* materialData)
	{
		using ResType = ConstructableBindable::ResourceType;
		uint32_t size = asset["Materials"].Size();
		for (ElementRef& mat : asset["Materials"].AsStruct())
		{
			materialData->hasDiffuseMap = mat["HasDiffuse"];
			materialData->hasSpecular = mat["HasSpecular"];
			materialData->hasNormalMap = mat["HasNormal"];

			materialData->m_Techniques.reserve(mat["Techniques"].Size());

			for (ElementRef& technique : mat["Techniques"].AsStruct())
			{
				TechniqueData techData = TechniqueData(technique.m_MetaElement.m_Name);
				StepData curStep = StepData((uint16_t)technique[0]["StepID"]);

				for (uint32_t i = 0; i < technique.Size(); i++)
				{
					ElementRef& bindable = technique[i];
					uint16_t stepID = bindable["StepID"];
					if (stepID != curStep.m_ID) 
					{
						techData.m_Steps.push_back(curStep);
						curStep = StepData(stepID);
					}

					ResType resType = (ResType)((int)bindable["ResType"]);
					bool shared = bindable["Shared"];
					ElementRef data = *bindable["Data"];

					switch(resType)
					{
					case ResType::VertexBuffer:
						assert("Vertex buffer not handled" && false);
						/*if (shared)
						{
							Ref<SharedBindable> vertBuf = CreateRef<SharedBindable>(ResType::VertexBuffer, (char*)data["Path"]);
							vertBuf->Initialize<VertexBuffer>();
							curStep.AddBindable(vertBuf);
						}
						else
						{
							Ref<UniqueBindable> vertBuf = CreateRef<UniqueBindable>(ResType::VertexBuffer);
							vertBuf->Initialize<VertexBuffer>();
							curStep.AddBindable(vertBuf);
						}*/
						break;
					case ResType::IndexBuffer:
						assert("Index buffer not handled" && false);
						/*if (shared)
						{
							Ref<SharedBindable> indexBuf = CreateRef<SharedBindable>(ResType::IndexBuffer, (char*)data["Path"]);
							indexBuf->Initialize<IndexBuffer>();
							curStep.AddBindable(indexBuf);
						}
						else
						{
							Ref<UniqueBindable> indexBuf = CreateRef<UniqueBindable>(ResType::IndexBuffer);
							indexBuf->Initialize<IndexBuffer>();
							curStep.AddBindable(indexBuf);
						}*/
						break;
					case ResType::VertexConstantBuffer:
						assert("Vertex constant buffer not handled" && false);
						/*if (shared)
						{
							Ref<SharedBindable> vertConstBuf = CreateRef<SharedBindable>(ResType::VertexConstantBuffer, (char*)data["Path"]);
							vertConstBuf->Initialize<VertexConstantBuffer>((int)data["Slot"]);
							curStep.AddBindable(vertConstBuf);
						}
						else
						{
							Ref<UniqueBindable> vertConstBuf = CreateRef<UniqueBindable>(ResType::VertexConstantBuffer);
							vertConstBuf->Initialize<VertexConstantBuffer>();
							curStep.AddBindable(vertConstBuf);
						}*/
						break;
					case ResType::PixelConstantBuffer:
						if (shared)
						{
							assert("Shared Pixel Constant Buffers not handled");
						}
						else
						{
							Ref<UniqueBindable> pixConstBuf = CreateRef<UniqueBindable>(ResType::PixelConstantBuffer);
							void* dataPtr;
							uint32_t dataSize;

							if (data.Has("SpecIntensity"))
							{
								struct PSMaterialConstant
								{
									float specularIntensity;
									float specularPower;
									BOOL hasAlphaGloss;
									float padding;
								};

								dataSize = sizeof(PSMaterialConstant);

								dataPtr = new PSMaterialConstant();
								PSMaterialConstant& pmc = *static_cast<PSMaterialConstant*>(dataPtr);

								pmc.specularIntensity = data["SpecIntensity"];
								pmc.specularPower = data["SpecPower"];
								pmc.hasAlphaGloss = data["HasAlphaGloss"];
							}
							else
							{
								namespace dx = DirectX;

								struct PSMaterialConstantNoTex
								{
									dx::XMFLOAT4 materialColor;
									dx::XMFLOAT4 specularColor;
									float specularPower;
									float padding[3];
								};

								dataSize = sizeof(PSMaterialConstantNoTex);

								dataPtr = new PSMaterialConstantNoTex();
								PSMaterialConstantNoTex& pmc = *static_cast<PSMaterialConstantNoTex*>(dataPtr);

								pmc.materialColor = data["MaterialCol"];
								pmc.specularColor = data["SpecCol"];
								pmc.specularPower = data["SpecPow"];
							}
							
							pixConstBuf->Initialize<PixelConstantBuffer>((int)data["Slot"], dataSize, dataPtr);
							curStep.AddBindable(pixConstBuf);
							delete dataPtr;
						}
						break;
					case ResType::Sampler:
						if (shared)
						{
							Ref<SharedBindable> sampler = CreateRef<SharedBindable>(ResType::Sampler, (char*)data);
							sampler->Initialize<Sampler>();
							curStep.AddBindable(sampler);
						}
						else
						{
							Ref<UniqueBindable> sampler = CreateRef<UniqueBindable>(ResType::Sampler);
							sampler->Initialize<Sampler>();
							curStep.AddBindable(sampler);
						}
						break;
					case ResType::PixelShader:
						if (shared)
						{
							Ref<SharedBindable> pixShader = CreateRef<SharedBindable>(ResType::PixelShader, (std::string)data);
							pixShader->Initialize<PixelShader>();
							curStep.AddBindable(pixShader);
						}
						else
						{
							Ref<UniqueBindable> pixShader = CreateRef<UniqueBindable>(ResType::PixelShader);
							pixShader->Initialize<PixelShader>((std::string)data);
							curStep.AddBindable(pixShader);
						}
						break;
					case ResType::VertexShader:
						if (shared)
						{
							Ref<SharedBindable> vertShader = CreateRef<SharedBindable>(ResType::VertexShader, (std::string)data);
							vertShader->Initialize<VertexShader>();
							curStep.AddBindable(vertShader);
						}
						else
						{
							Ref<UniqueBindable> vertShader = CreateRef<UniqueBindable>(ResType::VertexShader);
							vertShader->Initialize<VertexShader>((std::string)data);
							curStep.AddBindable(vertShader);
						}
						break;
					case ResType::Texture2D:
						if (shared)
						{
							Ref<SharedBindable> texture = CreateRef<SharedBindable>(ResType::Texture2D, (char*)data["Path"]);
							texture->Initialize<Texture2D>((int)data["Slot"]);
							curStep.AddBindable(texture);
						}
						else
						{
							Ref<UniqueBindable> texture = CreateRef<UniqueBindable>(ResType::Texture2D);
							texture->Initialize<Texture2D>((char*)data["Path"], (int)data["Slot"]);
							curStep.AddBindable(texture);
						}
						break;
					case ResType::Topology:
						assert("Topology not handled" && false);
						/*if (shared)
						{
							Ref<SharedBindable> topology = CreateRef<SharedBindable>(ResType::Topology, (char*)data["Path"]);
							topology->Initialize<Topology>();
							curStep.AddBindable(topology);
						}
						else
						{
							Ref<UniqueBindable> topology = CreateRef<UniqueBindable>(ResType::Topology);
							topology->Initialize<Topology>();
							curStep.AddBindable(topology);
						}*/
						break;
					case ResType::Blender:
						if (shared)
						{
							Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, (char*)data["Tag"]);
							blender->Initialize<Blender>((bool)data["Blending"]);
							curStep.AddBindable(blender);
						}
						else
						{
							assert("Not handled" && false);
							Ref<UniqueBindable> blender = CreateRef<UniqueBindable>(ResType::Blender);
							blender->Initialize<Blender>((bool)data["Blending"]);
							curStep.AddBindable(blender);
						}
					case ResType::Rasterizer:
						if (shared)
						{
							Ref<SharedBindable> rasterizer = CreateRef<SharedBindable>(ResType::Rasterizer, (char*)data["Tag"]);
							rasterizer->Initialize<Rasterizer>((bool)data["TwoSided"]);
							curStep.AddBindable(rasterizer);
						}
						else
						{
							assert("Not handled" && false);
							Ref<UniqueBindable> rasterizer = CreateRef<UniqueBindable>(ResType::Rasterizer);
							rasterizer->Initialize<Rasterizer>((bool)data["TwoSided"]);
							curStep.AddBindable(rasterizer);
						}
					default:
						assert("Unhandled resource type");
					}
				}

				techData.m_Steps.push_back(curStep);
				materialData->m_Techniques.push_back(techData);
			}

			materialData++;
		}
	}
	
	void ModelCreator::DeserializeNodes(Asset& asset, NodeData* nodeData, MeshData* meshData)
	{
		NodeData* nextNode;
		for (ElementRef& element : asset["Nodes"].AsStruct())
		{
			nextNode = nodeData + (uint32_t)element["Index"];

			assert("Node Data at position already initialized" && (nextNode->m_Name == ""));

			nextNode->m_Name = element.m_MetaElement.m_Name;
			nextNode->m_Transformation = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)element["TransformationMat"].m_Data);
		
			if (element.Has("Meshes"))
			{
				ElementRef meshes = element["Meshes"];
				nextNode->m_Meshes.reserve(meshes.Size());

				for (uint32_t i = 0; i < meshes.Size(); i++)
				{
					nextNode->m_Meshes.push_back((uint32_t)meshes[i]);
				}
			}

			if (element.Has("ChildNodes"))
			{
				ElementRef childNodes = element["ChildNodes"];
				nextNode->m_Children.reserve(childNodes.Size());

				for (uint32_t i = 0; i < childNodes.Size(); i++)
				{
					nextNode->m_Children.push_back((uint32_t)childNodes[i]);
				}
			}
		}
	}
}