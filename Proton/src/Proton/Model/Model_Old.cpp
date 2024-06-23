#include "ptpch.h"
#include "Model.h"
#include "Proton\Renderer\Bindables\Buffer.h"
#include "Proton\Renderer\Bindables\Texture.h"
#include "Proton\Renderer\Bindables\Sampler.h"
#include "Proton\Renderer\Bindables\Topology.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <unordered_map>
#include <imgui.h>
#include <filesystem>

#include "Proton\Scene\Scene.h"
#include "Proton\Scene\Entity.h"

#include "Proton\Asset Loader\AssetManager.h"

namespace Proton
{
	std::unordered_map<UUID, Ref<Model>> ModelCollection::m_Models = std::unordered_map<UUID, Ref<Model>>();

	/*Ref<Model> ModelCreator::GetModelFromData(ModelData& modelData)
	{
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

		return model;
	}

	static 	Entity CreateNode(NodeData& nodeData, NodeData* nodes, Mesh* meshes, Ref<Model> modelRef, Scene& activeScene)
	{
		//Node Creations
		namespace dx = DirectX;
		Entity childEntity = activeScene.CreateEntity(nodeData.m_Name);
		UUID childID = childEntity.GetUUID();
		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.NodeName = nodeData.m_Name;
		nodeComponent.Origin = nodeData.m_Transformation;

		nodeComponent.Children.reserve(nodeData.m_Children.size());

		MeshComponent& meshComponent = childEntity.AddComponent<MeshComponent>();
		meshComponent.ModelRef = modelRef;
		meshComponent.MeshPtrs.reserve(nodeData.m_Meshes.size());

		for (uint32_t meshIndex : nodeData.m_Meshes)
		{
			meshComponent.MeshPtrs.push_back(meshes + meshIndex);
		}

		for (uint32_t childIndex : nodeData.m_Children)
		{
			CreateNode(nodes[childIndex], nodes, meshes, modelRef, activeScene).SetParent(childID);
		}

		return childEntity;
	}

	Entity ModelCreator::CreateModelEntity(const std::string& path, Scene& activeScene)
	{
		namespace dx = DirectX;

		Ref<ModelData> modelData = AssetCollection::Get<ModelData>(path);

		NodeData& rootNode = modelData->m_Nodes[0];

		Ref<Model> model = GetModelFromData(*modelData);

		return CreateNode(rootNode, modelData->m_Nodes.data(), model->m_Meshes.data(), model, activeScene);
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

	Ref<ModelData> ModelCreator::Serialize(const std::string& path)
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(path.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace);

		Ref<ModelData> modelData = CreateRef<ModelData>();

		if (strlen(imp.GetErrorString()) > 0)
		{
			PT_CORE_ERROR("Error importing model!: {0}", std::string(imp.GetErrorString()));
			return modelData;
		}

		std::string basePath = std::filesystem::path(path).remove_filename().string();

		aiNode& node = *pScene->mRootNode;

		modelData->m_Meshes.resize(pScene->mNumMeshes);
		modelData->m_Materials.resize(pScene->mNumMaterials);

		RawAsset rawAsset = RawAsset();

		//TODO: Move to Asset Manager
		rawAsset.Add("AssetType", (uint32_t)AssetType::Model);

		rawAsset.Add("Materials", Type::Struct);

		for (uint32_t i = 0; i < pScene->mNumMaterials; i++)
		{
			SerializeMaterial(modelData->m_Materials.data() + i, rawAsset, basePath, i, *pScene->mMaterials[i]);
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
			TypeElement meshElement = std::move(SerializeMesh(modelData->m_Meshes.data() + i, rawAsset, basePath, path, *pScene->mMeshes[i], pScene, modelData->m_Materials.data()));
			rawAsset["Meshes"].Add(meshElement);
		}

		modelData->m_Nodes.resize(CountNodes(pScene->mRootNode));
		rawAsset.Add("Nodes", Type::Struct);

		uint32_t index = 0;
		NodeData* nodeData = modelData->m_Nodes.data();
		SerializeNode(nodeData, pScene->mRootNode, rawAsset, modelData->m_Meshes.data(), index);

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
		});//*//*

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
					Element::Create("X", diffuseColor.x),
					Element::Create("Y", diffuseColor.y),
					Element::Create("Z", diffuseColor.z),
					Element::Create("W", diffuseColor.w),
				}),
				Element("SpecCol", {
					Element::Create("X", specularColor.x),
					Element::Create("Y", specularColor.y),
					Element::Create("Z", specularColor.z),
					Element::Create("W", specularColor.w),
				}),
				Element::Create("SpecPow", shininess)
			});
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
				Element::Create("SpecIntensity", (float)buf["specularIntensity"]),
				Element::Create("SpecPower", (float)buf["specularPower"]),
				Element::Create("HasAlphaGloss", (bool)buf["hasAlphaGloss"]),
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

	Ref<ModelData> ModelCreator::Deserialize(Asset& modelAsset, const std::string& path)
	{
		Ref<ModelData> modelData = CreateRef<ModelData>();
		modelData->m_Materials.resize(modelAsset["Materials"].Size());
		modelData->m_Meshes.resize(modelAsset["Meshes"].Size());
		modelData->m_Nodes.resize(modelAsset["Nodes"].Size());

		DeserializeMaterials(modelAsset, modelData->m_Materials.data());

		DeserializeMeshes(modelAsset, path, modelData->m_Meshes.data(), modelData->m_Materials.data());

		DeserializeNodes(modelAsset, modelData->m_Nodes.data(), modelData->m_Meshes.data());

		return modelData;
	}

	void ModelCreator::DeserializeMeshes(Asset& asset, const std::string& modelPath, MeshData* meshData, const MaterialData* materials)
	{
		using ResType = ConstructableBindable::ResourceType;
		for (uint32_t i = 0; i < asset["Meshes"].Size(); i++, meshData++)
		{
			ElementRef& mesh = asset["Meshes"][i];
			std::string meshTag = modelPath + "%" + (const char*)*mesh["Name"];
			meshData->Create((const char*)*mesh["Name"], modelPath);
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
						PT_CORE_ASSERT(false, "Vertex buffer not handled");
						break;
					case ResType::IndexBuffer:
						PT_CORE_ASSERT(false, "Index buffer not handled");
						break;
					case ResType::VertexConstantBuffer:
						PT_CORE_ASSERT(false, "Vertex constant buffer not handled");
						break;
					case ResType::PixelConstantBuffer:
						if (shared)
						{
							PT_CORE_ASSERT(false, "Shared Pixel Constant Buffers not handled");
						}
						else
						{
							Ref<UniqueBindable> pixConstBuf = CreateRef<UniqueBindable>(ResType::PixelConstantBuffer);

							if (data.Has("SpecIntensity"))
							{
								DCB::RawLayout layout;
								layout.Add(DCB::Type::Float, "specularIntensity");
								layout.Add(DCB::Type::Float, "specularPower");
								layout.Add(DCB::Type::Bool, "hasAlphaGloss");

								pixConstBuf->Initialize<PixelConstantBuffer>((int)data["Slot"], DCB::CookedLayout(std::move(layout)));
								PixelConstantBuffer& buf = pixConstBuf->As<PixelConstantBuffer>();

								buf["specularIntensity"] = (float)data["SpecIntensity"];
								buf["specularPower"] = (float)data["SpecPower"];
								buf["hasAlphaGloss"] = (bool)data["HasAlphaGloss"];
							}
							else
							{
								namespace dx = DirectX;

								DCB::RawLayout layout;
								layout.Add(DCB::Type::Float4, "materialColor");
								layout.Add(DCB::Type::Float4, "specularColor");
								layout.Add(DCB::Type::Float, "specularPower");
								pixConstBuf->Initialize<PixelConstantBuffer>((int)data["Slot"], DCB::CookedLayout(std::move(layout)));
								PixelConstantBuffer& buf = pixConstBuf->As<PixelConstantBuffer>();

								buf["materialColor"] = (dx::XMFLOAT4)data["MaterialCol"];
								buf["specularColor"] = (dx::XMFLOAT4)data["SpecCol"];
								buf["specularPower"] = (float)data["SpecPow"];	
							}
							
							curStep.AddBindable(pixConstBuf);
						}
						break;
					case ResType::Sampler:
						if (shared)
						{
							Ref<SharedBindable> sampler = CreateRef<SharedBindable>(ResType::Sampler, (const char*)data);
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
							Ref<SharedBindable> pixShader = CreateRef<SharedBindable>(ResType::PixelShader, (const char*)data);
							pixShader->Initialize<PixelShader>();
							curStep.AddBindable(pixShader);
						}
						else
						{
							Ref<UniqueBindable> pixShader = CreateRef<UniqueBindable>(ResType::PixelShader);
							pixShader->Initialize<PixelShader>((const char*)data);
							curStep.AddBindable(pixShader);
						}
						break;
					case ResType::VertexShader:
						if (shared)
						{
							Ref<SharedBindable> vertShader = CreateRef<SharedBindable>(ResType::VertexShader, (const char*)data);
							vertShader->Initialize<VertexShader>();
							curStep.AddBindable(vertShader);
						}
						else
						{
							Ref<UniqueBindable> vertShader = CreateRef<UniqueBindable>(ResType::VertexShader);
							vertShader->Initialize<VertexShader>((const char*)data);
							curStep.AddBindable(vertShader);
						}
						break;
					case ResType::Texture2D:
						if (shared)
						{
							Ref<SharedBindable> texture = CreateRef<SharedBindable>(ResType::Texture2D, (const char*)data["Path"]);
							texture->Initialize<Texture2D>((int)data["Slot"]);
							curStep.AddBindable(texture);
						}
						else
						{
							Ref<UniqueBindable> texture = CreateRef<UniqueBindable>(ResType::Texture2D);
							texture->Initialize<Texture2D>((const char*)data["Path"], (int)data["Slot"]);
							curStep.AddBindable(texture);
						}
						break;
					case ResType::Topology:
						PT_CORE_ASSERT(false, "Topology not handled");
						break;
					case ResType::Blender:
						if (shared)
						{
							Ref<SharedBindable> blender = CreateRef<SharedBindable>(ResType::Blender, (const char*)data["Tag"]);
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
						break;
					case ResType::Rasterizer:
						if (shared)
						{
							Ref<SharedBindable> rasterizer = CreateRef<SharedBindable>(ResType::Rasterizer, (const char*)data["Tag"]);
							rasterizer->Initialize<Rasterizer>((bool)data["TwoSided"]);
							curStep.AddBindable(rasterizer);
						}
						else
						{
							PT_CORE_ASSERT(false, "Not handled");
							Ref<UniqueBindable> rasterizer = CreateRef<UniqueBindable>(ResType::Rasterizer);
							rasterizer->Initialize<Rasterizer>((bool)data["TwoSided"]);
							curStep.AddBindable(rasterizer);
						}
						break;
					default:
						PT_CORE_ASSERT(false, "Unhandled resource type");
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
	}*/
}