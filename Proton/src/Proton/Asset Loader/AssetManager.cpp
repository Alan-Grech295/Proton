#include "ptpch.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "AssetManager.h"
#include <Platform\DirectX 11\DirectXTexture.h>
#include "Proton\Model\Model.h"
#include "Proton\Scene\ModelCollection.h"
#include "Proton/Asset Loader/AssetSerializer.h"

namespace fs = std::filesystem;

namespace Proton
{
	AssetManager AssetManager::manager = AssetManager();

	AssetManager::AssetManager()
		:
		m_ProjectPath("")
	{
	}

	AssetManager::~AssetManager()
	{
	}

	void AssetManager::ScanProject()
	{
		ModelCreator::SerializeModel(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Models/nano_textured/nanosuit.obj");
		
		ModelCreator::DeserializeModel(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Models/nano_textured/nanosuit.obj");

		Asset& desAsset = AssetSerializer::DeserializeAsset(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Prefabs/asset.asset");

		std::string desName = desAsset["Name"];

		int arr1_0 = desAsset["Array1"][0];
		int arr1_1 = desAsset["Array1"][1];
		int arr1_2 = desAsset["Array1"][2];

		ElementRef& array9Ref = desAsset["Array9"];
		ElementRef& el1 = array9Ref[0];
		ElementRef& el2 = array9Ref[1];
		float dat = el2["Data3"]["Data2"];

		ElementRef& array10Ref = desAsset["Array10"];
		ElementRef& array2 = array10Ref[1];

		int second = array2[1];

		ElementRef& arr5Ref = desAsset["Array5"];
		ElementRef& struct2 = arr5Ref[0];
		std::string struct2Name = struct2["Name"];
		ElementRef& struct2Struct = struct2["Struct"];
		int struct2StructInt = struct2Struct["Int"];
		
		ElementRef& pointer = desAsset["Pointer"];
		std::string pointerData = *pointer;
		for (ElementRef& element : desAsset["Struct2"].AsStruct())
		{
			std::string name = element.m_MetaElement.m_Name;
		}

		for (ElementRef& element : desAsset["Struct3"].AsStruct())
		{
			std::string name = element.m_MetaElement.m_Name;
		}

		uint32_t structSize = desAsset["Struct2"].Size();
		uint32_t arraySize = desAsset["Array9"].Size();
		//*/

		//

		RawAsset rawAsset;
		rawAsset.Add("Age", 18);
		rawAsset.Add("Height", 178.12f);
		rawAsset.Add("Name", std::string("Alan"));
		rawAsset.Add("Pointer", Type::Pointer);
		rawAsset["Pointer"].SetPointer(Type::String, rawAsset);
		rawAsset["Pointer"]->SetData(std::string("Hello my name is Alan!"));

		//Test 1
		rawAsset.Add("Struct", Type::Struct);
		rawAsset["Struct"].Add("Number", 8712);
		rawAsset["Struct"].Add("Double", 8712.7612);
		rawAsset["Struct"].Add("String", std::string("Hello world"));

		//Test 2
		rawAsset.Add("Struct2", Type::Struct);
		rawAsset["Struct2"].Add("Number", 8712);
		rawAsset["Struct2"].Add("Double", 1654.62);
		rawAsset["Struct2"].Add("Array", Type::Array);
		rawAsset["Struct2"]["Array"].SetType(TypeElement(Type::Byte));
		rawAsset["Struct2"]["Array"].Add((byte)81);
		rawAsset["Struct2"]["Array"].Add((byte)255);
		rawAsset["Struct2"]["Array"].Add((byte)1);

		//Test 3
		rawAsset.Add("Struct3", Type::Struct);
		rawAsset["Struct3"].Add("Number", 8712);
		rawAsset["Struct3"].Add("Double", 1654.62);
		rawAsset["Struct3"].Add("Struct1", Type::Struct);
		rawAsset["Struct3"]["Struct1"].Add("Hello", std::string("Hello"));
		rawAsset["Struct3"]["Struct1"].Add("Struct2", Type::Struct);
		rawAsset["Struct3"]["Struct1"]["Struct2"].Add("Struct3", Type::Struct);
		rawAsset["Struct3"]["Struct1"]["Struct2"]["Struct3"].Add("Array", Type::Array);
		rawAsset["Struct3"]["Struct1"]["Struct2"]["Struct3"]["Array"].SetType(TypeElement(Type::Float));
		rawAsset["Struct3"]["Struct1"]["Struct2"]["Struct3"]["Array"].Add(812.3f);
		rawAsset["Struct3"]["Struct1"]["Struct2"]["Struct3"]["Array"].Add(82.32f);

		//Test 4
		rawAsset.Add("Array1", Type::Array);
		rawAsset["Array1"].SetType(TypeElement(Type::Int32));
		rawAsset["Array1"].Add(8712);
		rawAsset["Array1"].Add(120);
		rawAsset["Array1"].Add(9032);

		//Test 5
		rawAsset.Add("Array2", Type::Array);
		rawAsset["Array2"].SetType(TypeElement(Type::String));
		rawAsset["Array2"].Add(std::string("Hello"));
		rawAsset["Array2"].Add(std::string("My"));
		rawAsset["Array2"].Add(std::string("name"));
		rawAsset["Array2"].Add(std::string("is"));
		rawAsset["Array2"].Add(std::string("Alan"));

		//Test 6
		rawAsset.Add("Array3", Type::Array);
		rawAsset["Array3"].SetType(TypeElement(Type::Struct));
		rawAsset["Array3"].GetType().Add({
			Element("Data1", Type::Int32),
			Element("Data2", Type::Byte),
			});
		rawAsset["Array3"].Add(TypeElement({
			Element::Create("Data1", 1256),
			Element::Create("Data2", (byte)255),
			}));
		rawAsset["Array3"].Add(TypeElement({
			Element::Create("Data1", 461),
			Element::Create("Data2", (byte)12),
			}));

		rawAsset.Add("Array9", Type::Array);
		rawAsset["Array9"].SetType(TypeElement(Type::Struct));
		rawAsset["Array9"].GetType().Add({
			Element("Data1", Type::Int32),
			Element("Data2", Type::Float),
			Element("Data3", Type::Struct)
			});
		rawAsset["Array9"].GetType()["Data3"].Add({
			Element("Data1", Type::Int32),
			Element("Data2", Type::Float),
			});

		rawAsset["Array9"].Add(TypeElement({
			Element::Create("Data1", 1256),
			Element::Create("Data2", 712.2313f),
			Element("Data3", {
				Element::Create("Data1", -71),
				Element::Create("Data2", 612.3f),
			})
			}));
		rawAsset["Array9"].Add(TypeElement({
			Element::Create("Data1", 41),
			Element::Create("Data2", -7612.4f),
			Element("Data3", {
				Element::Create("Data1", 142),
				Element::Create("Data2", -7612.13f),
			})
			}));

		//Test 7
		rawAsset.Add("Array4", Type::Array);
		rawAsset["Array4"].SetType(TypeElement(Type::Struct));
		rawAsset["Array4"].GetType().Add({
			Element("Data1", Type::Int32),
			Element("Data2", Type::Byte),
			Element("Data3", Type::String)
			});
		rawAsset["Array4"].Add(TypeElement({
			Element::Create("Data1", 1256),
			Element::Create("Data2", (byte)255),
			Element::Create("Data3", std::string("Hello world"))
			}));
		rawAsset["Array4"].Add(TypeElement({
			Element::Create("Data1", 461),
			Element::Create("Data2", (byte)12),
			Element::Create("Data3", std::string("Hello world too!"))
		}));

		//Test 8
		rawAsset.Add("Array5", Type::Array);
		rawAsset["Array5"].SetType(TypeElement(Type::Struct));
		rawAsset["Array5"].GetType().Add({
			Element("Name", Type::String),
			Element("Struct", Type::Struct)
			});

		rawAsset["Array5"].GetType()["Struct"].Add({
			Element("Int", Type::Int32),
		});

		rawAsset["Array5"].Add(TypeElement({
			Element::Create("Name", std::string("Alan")),
			Element("Struct", {
				Element::Create("Int", 123)
			})
		}));

		rawAsset["Array5"].Add(TypeElement({
			Element::Create("Name", std::string("Bob")),
			Element("Struct", {
				Element::Create("Int", 62)
			})
		}));

		//Test 9
		rawAsset.Add("Array6", Type::Array);
		rawAsset["Array6"].SetType(Type::Array);
		rawAsset["Array6"].GetType().SetType(Type::Int16);
		TypeElement arr1 = TypeElement(Type::Array);
		arr1.SetType(Type::Int16);
		arr1.Add((int16_t)123);
		arr1.Add((int16_t)65000);
		arr1.Add((int16_t)712);
		arr1.Add((int16_t)591);
		arr1.Add((int16_t)54);
		rawAsset["Array6"].Add(arr1);

		TypeElement arr2 = TypeElement(Type::Array);
		arr2.SetType(Type::Int16);
		arr2.Add((int16_t)8172);
		arr2.Add((int16_t)4651);
		rawAsset["Array6"].Add(arr2);

		//Test 9
		rawAsset.Add("Array10", Type::Array);
		rawAsset["Array10"].SetType(Type::Array);
		rawAsset["Array10"].GetType().SetType(Type::Int32);
		arr1 = TypeElement(Type::Array);
		arr1.SetType(Type::Int32);
		arr1.Add(123);
		arr1.Add(65000);
		arr1.Add(712);
		rawAsset["Array10"].Add(arr1);

		arr2 = TypeElement(Type::Array);
		arr2.SetType(Type::Int32);
		arr2.Add(8172);
		arr2.Add(4651);
		arr2.Add(-12123);
		rawAsset["Array10"].Add(arr2);

		//Test 10
		rawAsset.Add("Array7", Type::Array);
		rawAsset["Array7"].SetType(Type::Array);
		rawAsset["Array7"].GetType().SetType(Type::Struct);
		rawAsset["Array7"].GetType().GetType().Add({
			Element("Data1", Type::Byte),
			Element("Data2", Type::Float)
			});

		TypeElement strArr1 = TypeElement(Type::Array);
		strArr1.SetType(Type::Struct);
		strArr1.GetType().Add({
			Element("Data1", Type::Byte),
			Element("Data2", Type::Float)
			});
		strArr1.Add(TypeElement({
			Element::Create("Data1", (byte)71),
			Element::Create("Data2", 712.62f)
			}));
		strArr1.Add(TypeElement({
			Element::Create("Data1", (byte)12),
			Element::Create("Data2", 876123.658123f)
			}));
		rawAsset["Array7"].Add(strArr1);

		TypeElement strArr2 = TypeElement(Type::Array);
		strArr2.SetType(Type::Struct);
		strArr2.GetType().Add({
			Element("Data1", Type::Byte),
			Element("Data2", Type::Float)
			});
		strArr2.Add(TypeElement({
			Element::Create("Data1", (byte)255),
			Element::Create("Data2", 712.62f)
			}));
		strArr2.Add(TypeElement({
			Element::Create("Data1", (byte)91),
			Element::Create("Data2", -812.612f)
			}));
		rawAsset["Array7"].Add(strArr2);

		//Test 11
		/*rawAsset.Add("Array8", Type::Array);
		rawAsset["Array8"].SetType(Type::Array);
		rawAsset["Array8"].GetType().SetSize(10).SetType(Type::Array);
		rawAsset["Array8"].GetType().GetType().SetSize(5).SetType(Type::Struct);
		rawAsset["Array8"].GetType().GetType().GetType().Add({
			Element("Data1", Type::Int32),
			Element("Data2", Type::Float)
			});*/

		//Continue test

		Asset asset(rawAsset);

		AssetSerializer::SerializeAsset(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Prefabs/asset.asset", asset);//*/

		//159 bytes, 25 elements

		manager.m_PostModelImports.clear();

		manager.ScanDirectory(manager.m_ProjectPath);

		for (auto& path : manager.m_PostImageImports)
		{
			File* file = manager.WriteImageData(path);

			file->WriteFile();

			delete file;
		}

		/*for (auto& pair : manager.m_PostModelReads)
		{
			std::string modelPath = fs::path(pair.first).replace_extension().string();

			File& file = *new File(pair.first, 0);
			file.ReadFile();

			file.Read<uint32_t>();

			manager.m_ModelAssets[modelPath] = manager.ImportModelAsset(file, fs::path(pair.first).remove_filename().string());

			//delete& file;
		}

		for (auto& path : manager.m_PostModelImports)
		{
			File* file = manager.WriteModelData(path);

			file->WriteFile();

			delete file;
		}

		for (auto& path : manager.m_PostPrefabReads)
		{
			File& file = *new File(path, 0);
			file.ReadFile();

			manager.m_Prefabs[path] = manager.ImportPrefab(file);

			delete &file;
		}*/
	}

	void AssetManager::SetProjectPath(const std::filesystem::path path)
	{
		manager.m_ProjectPath = path;
	}

	Ref<Image> AssetManager::GetImage(const std::string& path)
	{
		return manager.m_ImageAssets[path];
	}

	/*Ref<Model> AssetManager::GetModel(const std::string& path)
	{
		return manager.m_ModelAssets[path];
	}

	Ref<Prefab> AssetManager::GetPrefab(const std::string& path)
	{
		return manager.m_Prefabs[path];
	}*/

	void AssetManager::CreatePrefab(Entity& parentEntity, std::string savePath)
	{
		/*if (!parentEntity.HasComponent<RootNodeTag>())
		{
			assert(false && "Entity has to be a root node!");
			return;
		}

		if (fs::exists(savePath + ".prefab"))
		{
			uint32_t count = 1;
			while (fs::exists(savePath + std::to_string(count) + ".prefab"))
			{
				count++;
			}

			savePath = savePath + std::to_string(count) + ".prefab";
		}
		else
		{
			savePath = savePath + ".prefab";
		}

		PT_WARN(savePath);

		std::vector<std::string> modelPaths;

		WriteableBuffer* buffer = new WriteableBuffer();
		NodeComponent& parent = parentEntity.GetComponent<NodeComponent>();

		DirectX::XMFLOAT4X4 transformationMatrix;

		DirectX::XMStoreFloat4x4(&transformationMatrix, parent.m_Origin);

		buffer->WriteString(parentEntity.GetComponent<TagComponent>().tag);
		buffer->Write(transformationMatrix);
		buffer->Write((uint32_t)parent.m_ChildNodes.size());

		if (parentEntity.HasComponent<MeshComponent>())
		{
			MeshComponent& mesh = parentEntity.GetComponent<MeshComponent>();

			buffer->Write(mesh.m_NumMeshes);

			for (int i = 0; i < mesh.m_NumMeshes; i++)
			{
				Mesh& curMesh = *mesh.m_MeshPtrs[i];

				uint32_t modelID = manager.GetOrAdd(modelPaths, curMesh.m_ModelPath);

				buffer->Write(modelID);
				buffer->WriteString(curMesh.m_Name);
			}
		}
		else
		{
			buffer->Write((uint32_t)0);
		}

		uint32_t nodeIndex = 0;
		for (int i = 0; i < parent.m_ChildNodes.size(); i++)
		{
			nodeIndex++;
			manager.WriteNode(buffer, modelPaths, 0, nodeIndex, parent.m_ChildNodes[i]);
		}

		uint64_t fileSize = buffer->size() + 8;

		for (std::string str : modelPaths)
		{
			fileSize += 5 + str.length();
		}

		File& file = *new File(savePath, fileSize);

		file.Write((uint32_t)modelPaths.size());

		for (std::string str : modelPaths)
		{
			file.WriteString(str);
		}

		uint32_t bufferSize = buffer->size();
		file.Write(buffer->GetBuffer(), bufferSize, false);

		file.WriteFile();

		manager.m_Prefabs[savePath] = manager.CreatePrefabFromEntity(savePath, parentEntity);

		delete &file;*/
	}

	/*Ref<Prefab> AssetManager::ImportPrefab(File& file)
	{
		std::vector<std::string> modelPaths;
		uint32_t numModelPaths = file.Read<uint32_t>();

		for (int i = 0; i < numModelPaths; i++)
		{
			modelPaths.push_back(file.ReadString());
		}

		Ref<Prefab> prefab = CreateRef<Prefab>();
		std::vector<PrefabNode*> nodes;

		prefab->rootNode = new PrefabNode();

		prefab->rootNode->name = file.ReadString();
		prefab->rootNode->transformation = DirectX::XMLoadFloat4x4(&file.Read<DirectX::XMFLOAT4X4>());
		prefab->rootNode->childNodes = new PrefabNode*[file.Read<uint32_t>()];

		//Reading root node
		uint32_t numMeshes = file.Read<uint32_t>();
		prefab->rootNode->numMeshes = numMeshes;
		prefab->rootNode->meshes = new Mesh*[numMeshes];
		prefab->rootNode->numChildren = 0;
		prefab->rootNode->position = { 0, 0, 0 };
		prefab->rootNode->rotation = { 0, 0, 0 };
		prefab->rootNode->scale = { 1, 1, 1 };
		
		for (int i = 0; i < numMeshes; i++)
		{
			uint32_t modelID = file.Read<uint32_t>();
			std::string meshName = file.ReadString();

			Mesh* mesh = ModelCollection::GetMesh(modelPaths[modelID] + "%" + meshName);
			prefab->rootNode->meshes[i] = mesh;
		}

		ModelCollection::AddPrefabNode(file.GetPath().string(), prefab->rootNode->name, prefab->rootNode);

		nodes.push_back(prefab->rootNode);

		while (!file.ReachedEnd())
		{
			PrefabNode* childNode = new PrefabNode();

			childNode->name = file.ReadString();
			uint32_t parentIndex = file.Read<uint32_t>();
			childNode->transformation = DirectX::XMLoadFloat4x4(&file.Read<DirectX::XMFLOAT4X4>());
			childNode->childNodes = new PrefabNode*[file.Read<uint32_t>()];
			childNode->numChildren = 0;

			childNode->position = file.Read<DirectX::XMFLOAT3>();
			childNode->rotation = file.Read<DirectX::XMFLOAT3>();
			childNode->scale = file.Read<DirectX::XMFLOAT3>();

			uint32_t numMeshes = file.Read<uint32_t>();
			childNode->numMeshes = numMeshes;
			childNode->meshes = new Mesh*[numMeshes];

			for (int i = 0; i < numMeshes; i++)
			{
				uint32_t modelID = file.Read<uint32_t>();
				std::string meshName = file.ReadString();

				Mesh* mesh = ModelCollection::GetMesh(modelPaths[modelID] + "%" + meshName);
				childNode->meshes[i] = mesh;
			}

			nodes[parentIndex]->childNodes[nodes[parentIndex]->numChildren] = childNode;
			nodes[parentIndex]->numChildren++;
			ModelCollection::AddPrefabNode(file.GetPath().string(), childNode->name, childNode);

			nodes.push_back(childNode);
		}

		return prefab;
	}

	Ref<Prefab> AssetManager::CreatePrefabFromEntity(std::string prefabPath, Entity& parentEntity)
	{
		Ref<Prefab> prefab = CreateRef<Prefab>();

		prefab->rootNode = new PrefabNode();

		NodeComponent& root = parentEntity.GetComponent<NodeComponent>();

		prefab->rootNode->name = root.m_NodeName;
		prefab->rootNode->transformation = root.m_Origin;
		prefab->rootNode->childNodes = new PrefabNode*[root.m_ChildNodes.size()];

		//Reading root node
		uint32_t numMeshes = parentEntity.HasComponent<MeshComponent>() ? parentEntity.GetComponent<MeshComponent>().m_NumMeshes : 0;
		prefab->rootNode->numMeshes = numMeshes;
		prefab->rootNode->meshes = new Mesh * [numMeshes];
		prefab->rootNode->numChildren = root.m_ChildNodes.size();
		prefab->rootNode->position = { 0, 0, 0 };
		prefab->rootNode->rotation = { 0, 0, 0 };
		prefab->rootNode->scale = { 1, 1, 1 };

		if (parentEntity.HasComponent<MeshComponent>())
		{
			MeshComponent& mesh = parentEntity.GetComponent<MeshComponent>();
			memcpy(prefab->rootNode->meshes, mesh.m_MeshPtrs.data(), mesh.m_MeshPtrs.size() * sizeof(Mesh*));
			ModelCollection::AddPrefabNode(prefabPath, prefab->rootNode->name, prefab->rootNode);
		}

		for (int i = 0; i < root.m_ChildNodes.size(); i++)
		{
			prefab->rootNode->childNodes[i] = CreateChildPrefab(prefabPath, root.m_ChildNodes[i]);
		}

		return prefab;
	}

	PrefabNode* AssetManager::CreateChildPrefab(std::string prefabPath, Entity& childEntity)
	{
		PrefabNode* node = new PrefabNode();
		TransformComponent& transformComponent = childEntity.GetComponent<TransformComponent>();
		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();

		node->name = nodeComponent.m_NodeName;
		node->transformation = nodeComponent.m_Origin;
		node->childNodes = new PrefabNode*[nodeComponent.m_ChildNodes.size()];
		node->numChildren = nodeComponent.m_ChildNodes.size();

		node->position = transformComponent.position;
		node->rotation = transformComponent.rotation;
		node->scale = transformComponent.scale;

		node->numMeshes = 0;

		if (childEntity.HasComponent<MeshComponent>())
		{
			MeshComponent& mesh = childEntity.GetComponent<MeshComponent>();
			node->numMeshes = mesh.m_NumMeshes;
			node->meshes = new Mesh * [mesh.m_NumMeshes];
			memcpy(node->meshes, mesh.m_MeshPtrs.data(), mesh.m_MeshPtrs.size() * sizeof(Mesh*));

			ModelCollection::AddPrefabNode(prefabPath, node->name, node);
		}
		
		for (int i = 0; i < nodeComponent.m_ChildNodes.size(); i++)
		{
			node->childNodes[i] = CreateChildPrefab(prefabPath, nodeComponent.m_ChildNodes[i]);
		}

		return node;
	}

	void AssetManager::WriteNode(WriteableBuffer* buffer, std::vector<std::string>& modelPaths, uint32_t parentIndex, uint32_t& nodeIndex, Entity& entity)
	{
		TransformComponent& transform = entity.GetComponent<TransformComponent>();
		NodeComponent& child = entity.GetComponent<NodeComponent>();

		DirectX::XMFLOAT4X4 transformationMatrix;

		DirectX::XMStoreFloat4x4(&transformationMatrix, child.m_Origin);

		buffer->WriteString(entity.GetComponent<TagComponent>().tag);
		buffer->Write(parentIndex);
		buffer->Write(transformationMatrix);
		buffer->Write((uint32_t)child.m_ChildNodes.size());
		buffer->Write(transform.position);
		buffer->Write(transform.rotation);
		buffer->Write(transform.scale);

		if (entity.HasComponent<MeshComponent>())
		{
			MeshComponent& childMesh = entity.GetComponent<MeshComponent>();

			buffer->Write(childMesh.m_NumMeshes);

			for (int j = 0; j < childMesh.m_NumMeshes; j++)
			{
				Mesh& curMesh = *childMesh.m_MeshPtrs[j];

				uint32_t modelID = GetOrAdd(modelPaths, curMesh.m_ModelPath);

				buffer->Write(modelID);
				buffer->WriteString(curMesh.m_Name);
			}
		}
		else
		{
			buffer->Write((uint32_t)0);
		}

		uint32_t currentIndex = nodeIndex;

		for (int i = 0; i < child.m_ChildNodes.size(); i++)
		{
			nodeIndex++;
			WriteNode(buffer, modelPaths, currentIndex, nodeIndex, child.m_ChildNodes[i]);
		}
	}*/

	void AssetManager::ScanDirectory(const std::filesystem::path& path)
	{
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_directory()) {
				ScanDirectory(entry);
			}
			else if (entry.path().has_extension())
			{
				HandleFile(entry);
			}
		}
	}

	void AssetManager::HandleFile(const std::filesystem::path& path)
	{
		/*if (path.extension() == ".rawAsset")
		{
			fs::path pathCopy = fs::path(path);
			if (!fs::exists(pathCopy.replace_extension()))
			{
				remove(path);
			}

			char* data = AssetLoader::ReadData(path);

			uint64_t ptr = 0;
			AssetType type = *(AssetType*)&data[ptr];
			ptr += sizeof(type);

			switch (type)
			{
			case AssetType::Image:
				{
					File& file = *new File(path, 0);
					file.ReadFile();

					file.Read<uint32_t>();

					m_ImageAssets[pathCopy] = ImportImageAsset(file);
				}
				break;
			case AssetType::Model:
				m_PostModelReads.push_back({ path, &data[ptr] });
				break;
			}
		}

		if (path.extension() == ".prefab")
		{
			m_PostPrefabReads.push_back(path);
		}

		if (path.extension() == ".obj")
		{
			std::string outFileName(path.string() + std::string(".rawAsset"));

			if (fs::exists(fs::path(outFileName)))
				return;

			m_PostModelImports.push_back(path);
		}

		if (path.extension() == ".gltf")
		{
			std::string outFileName(path.string() + std::string(".rawAsset"));

			if (fs::exists(fs::path(outFileName)))
				return;

			m_PostModelImports.push_back(path);
		}

		if (path.extension() == ".png")
		{
			std::string outFileName(path.string() + std::string(".rawAsset"));

			if (fs::exists(fs::path(outFileName)))
				return;

			m_PostImageImports.push_back(path);
		}*/
	}

	/*File* AssetManager::WriteModelData(std::filesystem::path& modelPath)
	{
		namespace dx = DirectX;
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(modelPath.string().c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace);

		if (pScene == nullptr)
		{
			PT_CORE_ERROR("Error importing model!: {0}", std::string(imp.GetErrorString()));
		}

		aiNode& node = *pScene->mRootNode;

		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
			dx::XMFLOAT3 tangent;
			dx::XMFLOAT3 bitangent;
			dx::XMFLOAT2 uv;
		};

		std::vector<std::vector<Vertex>> allVerts;
		std::vector<std::vector<uint32_t>> allIndices;

		allVerts.resize(pScene->mNumMeshes);
		allIndices.resize(pScene->mNumMeshes);

		uint64_t bufferSize = 0;
		bufferSize += sizeof(uint32_t);					//rawAsset Type
		bufferSize += sizeof(pScene->mNumMeshes);			//No. of meshes

		Ref<Model> model = CreateRef<Model>();
		Mesh** meshes = new Mesh*[pScene->mNumMeshes];

		std::string basePath = std::filesystem::path(modelPath).remove_filename().string();

		for (int i = 0; i < pScene->mNumMeshes; i++)
		{
			aiMesh& mesh = *pScene->mMeshes[i];

			bufferSize += sizeof(uint32_t) + mesh.mName.length + 1;

			meshes[i] = ModelCreator::SerializeMesh(basePath, modelPath.string(), mesh, pScene->mMaterials);

			allVerts[i].reserve(mesh.mNumVertices);
			for (uint32_t j = 0; j < mesh.mNumVertices; j++)
			{
				allVerts[i].push_back({
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[j]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[j]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[j]),
					*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[j]),
					*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][j])
					});
			}

			bufferSize += sizeof(uint32_t) + sizeof(Vertex) * allVerts[i].size();		//Vertex data

			allIndices[i].reserve(mesh.mNumFaces * 3);
			for (unsigned int j = 0; j < mesh.mNumFaces; j++)
			{
				const auto& face = mesh.mFaces[j];
				assert(face.mNumIndices == 3);
				allIndices[i].push_back(face.mIndices[0]);
				allIndices[i].push_back(face.mIndices[1]);
				allIndices[i].push_back(face.mIndices[2]);
			}

			bufferSize += sizeof(uint32_t) + sizeof(uint32_t) * allIndices[i].size();	//Index data

			bufferSize += 15;

			if (mesh.mMaterialIndex >= 0)
			{
				auto& material = *pScene->mMaterials[mesh.mMaterialIndex];

				aiString texFileName;

				if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
					bufferSize += (basePath + texFileName.C_Str()).length() + 1;

				if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
					bufferSize += (basePath + texFileName.C_Str()).length() + 1;

				if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
					bufferSize += (basePath + texFileName.C_Str()).length() + 1;
			}

			bufferSize += 7 * sizeof(float);
		}

		model->rootNode = CreateNodeTree(*pScene->mRootNode, meshes);

		uint32_t numChildren = 0;
		uint32_t nodeBufferSize = 0;

		std::vector<uint32_t> nodesPerLevel;

		NodeTreeSizeCheck(*pScene->mRootNode, nodeBufferSize, numChildren);
		bufferSize += nodeBufferSize + sizeof(uint32_t);

		PT_CORE_TRACE("Writing Model {0}", modelPath.string());
		File* file = new File(modelPath.string() + std::string(".rawAsset"), bufferSize);

		file->Write((uint32_t)AssetType::Model);

		file->Write(pScene->mNumMeshes);

		for (int i = 0; i < pScene->mNumMeshes; i++)
		{
			aiMesh& mesh = *pScene->mMeshes[i];

			file->WriteString(std::string(mesh.mName.C_Str()));

			uint32_t numVertBytes = sizeof(Vertex) * allVerts[i].size();
			file->Write(allVerts[i].data(), numVertBytes);

			uint32_t numIndexBytes = sizeof(uint32_t) * allIndices[i].size();
			file->Write(allIndices[i].data(), numIndexBytes);

			std::string diffuseAssetPath = "";
			std::string specularAssetPath = "";
			std::string normalsAssetPath = "";
			aiColor3D diffuseColor = { 0, 0, 0 };
			aiColor3D specularColor = { 0, 0, 0 };
			float shininess = 0;

			if (mesh.mMaterialIndex >= 0)
			{
				auto& material = *pScene->mMaterials[mesh.mMaterialIndex];

				aiString texFileName;

				if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
					diffuseAssetPath = basePath + texFileName.C_Str();
				
				material.Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);

				if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
					specularAssetPath = basePath + texFileName.C_Str();
				
				material.Get(AI_MATKEY_COLOR_SPECULAR, specularColor);

				material.Get(AI_MATKEY_SHININESS, shininess);

				if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
					normalsAssetPath = basePath + texFileName.C_Str();
			}

			file->WriteString(diffuseAssetPath);

			file->WriteString(specularAssetPath);

			file->WriteString(normalsAssetPath);

			file->Write(diffuseColor);

			file->Write(specularColor);

			file->Write(shininess);
		}

		std::vector<uint32_t> flattenedTree;
		flattenedTree.resize(numChildren);
		flattenedTree.reserve(nodeBufferSize * 0.25f);
		uint32_t nodeIndex = 0;
		AddNodeTree(*pScene->mRootNode, nodeIndex, flattenedTree);

		uint32_t treeSize = flattenedTree.size() * sizeof(uint32_t);
		file->Write(flattenedTree.data(), treeSize);

		file->ShowSize();
		
		m_ModelAssets[modelPath] = model;

		delete[] meshes;

		return file;
	}*/

	File* AssetManager::WriteImageData(const std::filesystem::path& imagePath)
	{
		DirectX::ScratchImage scratch;
		HRESULT hr = DirectX::LoadFromWICFile(imagePath.wstring().c_str(), DirectX::WIC_FLAGS_NONE,
			nullptr, scratch);

		if (FAILED(hr))
		{
			PT_CORE_ERROR("[TEXTURE PATH] {0}", imagePath.string());
			GET_ERROR(hr);
		}

		if (scratch.GetImage(0, 0, 0)->format != DirectXTexture2D::format)
		{
			DirectX::ScratchImage converted;
			hr = DirectX::Convert(
				*scratch.GetImage(0, 0, 0),
				DirectXTexture2D::format,
				DirectX::TEX_FILTER_DEFAULT,
				DirectX::TEX_THRESHOLD_DEFAULT,
				converted
			);

			scratch = std::move(converted);
		}

		uint32_t width = scratch.GetMetadata().width;
		uint32_t height = scratch.GetMetadata().height;
		bool isOpaque = scratch.IsAlphaAllOpaque();

		PT_CORE_TRACE("Writing Image {0}", imagePath.string());
		File* file = new File(imagePath.string() + std::string(".rawAsset"), scratch.GetPixelsSize() + 18 + imagePath.string().length());

		//file->Write((uint32_t)AssetType::Image);

		file->Write(width);

		file->Write(height);

		file->Write(isOpaque);

		uint32_t pixels = scratch.GetPixelsSize();
		file->Write(scratch.GetPixels(), pixels);

		Ref<Image> img = CreateRef<Image>();
		img->width = width;
		img->height = height;
		img->isOpaque = isOpaque;
		img->pixels = new uint8_t[pixels];
		memcpy(img->pixels, scratch.GetPixels(), pixels);

		m_ImageAssets[imagePath] = img; 

		return file;
	}

	/*Ref<Model> AssetManager::ImportModelAsset(File& file, const std::string& basePath)
	{
		//TODO: Change model import and export
		namespace dx = DirectX;
		Ref<Model> modelAsset = CreateRef<Model>();

		uint32_t numMeshes = file.Read<uint32_t>();

		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
			dx::XMFLOAT3 tangent;
			dx::XMFLOAT3 bitangent;
			dx::XMFLOAT2 uv;
		};

		Mesh** meshes = new Mesh*[numMeshes];

		for (int i = 0; i < numMeshes; i++)
		{
			std::string name = file.ReadString();

			PT_CORE_TRACE("Imported {0}", std::string(name));

			uint32_t numVertBytes;
			Vertex* vertices = file.ReadArray<Vertex>(numVertBytes);

			uint32_t numIndexBytes;
			uint32_t* indices = file.ReadArray<uint32_t>(numIndexBytes);

			std::string diffuseAssetPath = "";
			std::string specularAssetPath = "";
			std::string normalsAssetPath = "";
			aiColor3D diffuseColor = { 0, 0, 0 };
			aiColor3D specularColor = { 0, 0, 0 };
			float shininess = 0;

			diffuseAssetPath = file.ReadString();

			specularAssetPath = file.ReadString();

			normalsAssetPath = file.ReadString();

			diffuseColor = file.Read<aiColor3D>();

			specularColor = file.Read<aiColor3D>();

			shininess = file.Read<float>();

			auto meshTag = file.GetPath().replace_extension().string() + "%" + name;

			Mesh* mesh = new Mesh(meshTag, name, file.GetPath().replace_extension().string());
			mesh->hasDiffuseMap = diffuseAssetPath != "";
			mesh->hasSpecular = specularAssetPath != "";
			mesh->hasNormalMap = normalsAssetPath != "";
			
			bool hasAlphaGloss = false;

			//TODO: Change rawAsset loading
			//TEMP
			Technique technique = Technique("Opaque");
			Step step = Step("Lambertian");

			step.AddBindable(mesh->m_TransformCBuf);
			step.AddBindable(mesh->m_TransformCBufPix);

			if (mesh->hasDiffuseMap)
				step.AddBindable(Texture2D::Create(diffuseAssetPath));

			if (mesh->hasSpecular)
			{
				Ref<Texture2D> spec = Texture2D::Create(specularAssetPath, 1);
				hasAlphaGloss = spec->HasAlpha();
				step.AddBindable(spec);
			}

			if (mesh->hasNormalMap)
				step.AddBindable(Texture2D::Create(normalsAssetPath, 2));

			if (mesh->hasSpecular || mesh->hasNormalMap || mesh->hasDiffuseMap)
				step.AddBindable(Sampler::Create(meshTag));

			Ref<PixelShader> pixShader;
			Ref<VertexShader> vertShader;

			if (mesh->hasSpecular && !mesh->hasNormalMap)
			{
				pixShader = PixelShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongSpecularPS.cso");
				vertShader = VertexShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongNormalMapVS.cso");
			}
			else if (mesh->hasNormalMap && !mesh->hasSpecular)
			{
				pixShader = PixelShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongNormalMapPS.cso");
				vertShader = VertexShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongNormalMapVS.cso");
			}
			else if (mesh->hasNormalMap && mesh->hasSpecular)
			{
				pixShader = PixelShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongNormalMapSpecPS.cso");
				vertShader = VertexShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongNormalMapVS.cso");
			}
			else if (!mesh->hasNormalMap && !mesh->hasSpecular && mesh->hasDiffuseMap)
			{
				pixShader = PixelShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongPS.cso");
				vertShader = VertexShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongVS.cso");
			}
			else
			{
				pixShader = PixelShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongNoTexPS.cso");
				vertShader = VertexShader::Create(manager.m_ProjectPath.generic_string() + "\\Proton\\PhongVS.cso");
			}

			step.AddBindable(vertShader);
			step.AddBindable(pixShader);

			mesh->m_IndexBuffer = IndexBuffer::Create(meshTag);
			mesh->m_IndexBuffer->SetRawData(indices, numIndexBytes / sizeof(uint32_t));

			BufferLayout layout = {
				{"POSITION", ShaderDataType::Float3},
				{"NORMAL", ShaderDataType::Float3},
				{"TANGENT", ShaderDataType::Float3},
				{"BITANGENT", ShaderDataType::Float3},
				{"TEXCOORD", ShaderDataType::Float2}
			};

			//TODO: Set actually dynamic vertices
			//Currently the vertex data is passed directly to the buffer, but in the future
			//will have to filter out certain data
			mesh->m_VertBuffer = VertexBuffer::Create(meshTag, layout, vertShader.get());
			mesh->m_VertBuffer->SetRawData((char*)vertices, numVertBytes);

			mesh->m_Topology = Topology::Create(TopologyType::TriangleList);

			if (!mesh->hasDiffuseMap && !mesh->hasSpecular && !mesh->hasNormalMap)
			{
				struct PSMaterialConstantNoTex
				{
					dx::XMFLOAT4 materialColor;
					dx::XMFLOAT4 specularColor;
					float specularPower;
					float padding[3];
				} pmc;

				pmc.specularPower = shininess;
				pmc.specularColor = { specularColor.r, specularColor.g, specularColor.b, 1.0f};
				pmc.materialColor = { diffuseColor.r, diffuseColor.g, diffuseColor.b, 1.0f };
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
				pmc.specularIntensity = (specularColor.r + specularColor.g + specularColor.b) / 3.0f;
				pmc.hasAlphaGloss = hasAlphaGloss ? TRUE : FALSE;
				step.AddBindable(PixelConstantBuffer::CreateUnique(1, sizeof(pmc), &pmc));
			}

			meshes[i] = mesh;
			technique.AddStep(step);
			mesh->m_Techniques.push_back(technique);

			ModelCollection::AddMesh(meshTag, mesh);
		}

		uint32_t treeSize;
		uint32_t* data = file.ReadArray<uint32_t>(treeSize);

		modelAsset->rootNode = ReadNodeTree(data, meshes, 0);

		delete[] meshes;

		return modelAsset;
	}*/

	Ref<Image> AssetManager::ImportImageAsset(File& file)
	{
		Ref<Image> imageAsset = CreateRef<Image>();

		imageAsset->width = file.Read<uint32_t>();

		imageAsset->height = file.Read<uint32_t>();

		imageAsset->isOpaque = file.Read<bool>();

		uint32_t arraySize;
		uint8_t* pixels = file.ReadArray<uint8_t>(arraySize);
		imageAsset->pixels = new uint8_t[arraySize];
		//memcpy(imageAsset->pixels, pixels, arraySize);
		imageAsset->pixels = pixels;
		imageAsset->pixelSize = arraySize;

		return imageAsset;
	}

	/*void AssetManager::NodeTreeSizeCheck(aiNode& node, uint32_t& bufferSize, uint32_t& numChildren)
	{
		//Need to add:
		//		uint32_t for node location
		//		uint32_t for child amnt
		//		uint32_t for mesh amnt
		//		uint32_t for name length
		//		As many uint32_t's as name characters
		//		Matrix for transformation
		//		As many uint32_t's as children
		//		As many uint32_t's as meshes
		numChildren++;
		bufferSize += (4 + node.mNumChildren + node.mNumMeshes + node.mName.length + 1) * sizeof(uint32_t) + sizeof(DirectX::XMFLOAT4X4);

		for (int i = 0; i < node.mNumChildren; i++)
		{
			NodeTreeSizeCheck(*node.mChildren[i], bufferSize, numChildren);
		}
	}

	void AssetManager::AddNodeTree(aiNode& node, uint32_t& nodeIndex, std::vector<uint32_t>& tree)
	{
		tree[nodeIndex] = tree.size();
		tree.push_back(node.mName.length + 1);

		for (int i = 0; i < node.mName.length + 1; i++)
		{
			tree.push_back((uint32_t)node.mName.C_Str()[i]);
		}

		uint32_t* transformationMatrix = (uint32_t*)reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation);

		for (int i = 0; i < sizeof(DirectX::XMFLOAT4X4) / 4; i++)
		{
			tree.push_back(transformationMatrix[i]);
		}

		tree.push_back(node.mNumChildren);

		uint32_t childNodeStart = tree.size();

		for (int i = 0; i < node.mNumChildren; i++)
		{
			tree.push_back(0);
		}

		tree.push_back(node.mNumMeshes);

		for (int i = 0; i < node.mNumMeshes; i++)
		{
			tree.push_back(node.mMeshes[i]);
		}

		for (int i = 0; i < node.mNumChildren; i++)
		{
			tree[childNodeStart + i] = ++nodeIndex;
			AddNodeTree(*node.mChildren[i], nodeIndex, tree);
		}
	}

	Node* AssetManager::CreateNodeTree(aiNode& node, Mesh** meshes)
	{
		Node* createdNode = new Node();
		createdNode->childNodes = new Node*[node.mNumChildren];
		createdNode->numChildren = node.mNumChildren;
		createdNode->name = std::string(node.mName.C_Str());
		createdNode->transformation = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(
				reinterpret_cast<const DirectX::XMFLOAT4X4*>(&node.mTransformation)
			)
		);
		createdNode->numMeshes = node.mNumMeshes;
		createdNode->meshes = new Mesh*[node.mNumMeshes];

		for (int i = 0; i < node.mNumMeshes; i++)
		{
			createdNode->meshes[i] = meshes[node.mMeshes[i]];
		}

		for (int i = 0; i < node.mNumChildren; i++)
		{
			createdNode->childNodes[i] = CreateNodeTree(*node.mChildren[i], meshes);
		}

		return createdNode;
	}

	Node* AssetManager::ReadNodeTree(const uint32_t* data, Mesh** meshes, uint32_t nodeIndex)
	{
		uint32_t nodeDataPosition = data[nodeIndex];
		uint32_t nameLength = data[nodeDataPosition];

		std::string name = "";
		for (int i = 0; i < nameLength; i++)
		{
			name += (char)data[nodeDataPosition + 1 + i];
		}

		uint32_t matrixStart = nodeDataPosition + 1 + nameLength;
		float* transformationFloats = new float[16];

		for (int i = 0; i < (sizeof(DirectX::XMFLOAT4X4) / 4); i++)
		{
			transformationFloats[i] = *(float*)&data[matrixStart + i];
		}

		DirectX::XMMATRIX transformation = DirectX::XMMatrixTranspose(
			DirectX::XMLoadFloat4x4(
				reinterpret_cast<const DirectX::XMFLOAT4X4*>(transformationFloats)
			)
		);

		uint32_t childStart = matrixStart + (sizeof(DirectX::XMFLOAT4X4) / 4);
		uint32_t numChildren = data[childStart];

		Node* node = new Node();
		node->numChildren = numChildren;
		node->childNodes = new Node*[numChildren];
		node->name = name;
		node->transformation = transformation;

		for (int i = 0; i < numChildren; i++)
		{
			node->childNodes[i] = ReadNodeTree(data, meshes, data[childStart + 1 + i]);
		}

		uint32_t meshStart = childStart + 1 + numChildren;
		uint32_t numMeshes = data[meshStart];
		node->numMeshes = numMeshes;
		node->meshes = new Mesh*[numMeshes];

		for (int i = 0; i < numMeshes; i++)
		{
			node->meshes[i] = meshes[data[meshStart + 1 + i]];
		}

		return node;
	}*/

	File::File(std::filesystem::path path, uint64_t fileSize)
		:
		pointer(0),
		size(fileSize),
		filePath(path),
		buffer(new char[fileSize])
	{
	}

	File::~File()
	{
		delete[] buffer;
	}

	template<typename T>
	void File::Write(T& value)
	{
		if (std::is_same<T, std::string>::value)
			assert(false && "Use WriteString for strings!");

		memcpy(&buffer[pointer], &value, sizeof(T));
		pointer += sizeof(T);
	}

	template<typename T>
	void File::Write(T&& value)
	{
		T var = value;
		Write(var);
	}

	void File::WriteString(const std::string& value)
	{
		uint32_t length = (uint32_t)(value.length() + 1);
		Write((void*)value.c_str(), length);
	}

	void File::Write(void* value, uint32_t& size, bool writeSize)
	{
		if(writeSize) 
			Write(size);

		memcpy(&buffer[pointer], value, size);
		pointer += size;
	}

	template<typename T>
	T File::Read()
	{
		T data = *(T*)&buffer[pointer];
		pointer += sizeof(T);
		return data;
	}

	template<typename T>
	T* File::ReadArray(uint32_t& arraySize)
	{
		arraySize = Read<uint32_t>();
		T* data = (T*)&buffer[pointer];
		pointer += arraySize;
		return data;
	}

	template<typename T>
	T* File::GetEndArrayPtr()
	{
		return (T*)&buffer[pointer];
	}

	std::string File::ReadString()
	{
		uint32_t arraySize;
		char* stringPtr = ReadArray<char>(arraySize);
		return std::string(stringPtr);
	}

	void File::WriteFile()
	{
		std::ofstream outStream(filePath, std::ios::out | std::ios::binary);
		outStream.write(buffer, pointer);
		outStream.close();
	}

	void File::ReadFile()
	{
		std::ifstream inStream(filePath, std::ios::out | std::ios::binary);

		if (!inStream)
		{
			PT_ERROR("Cannot open file {0}!", filePath.string());
		}

		std::filebuf* fileBuf = inStream.rdbuf();
		size = fileBuf->pubseekoff(0, inStream.end, inStream.in);
		fileBuf->pubseekpos(0, inStream.in);

		buffer = new char[size];

		fileBuf->sgetn(buffer, size);

		PT_CORE_WARN("Read File {0}", filePath.string());

		inStream.close();
	}

	void File::ShowSize()
	{
	}

	WriteableBuffer::WriteableBuffer()
	{
	}

	WriteableBuffer::~WriteableBuffer()
	{
		delete[] buffer.data();
	}

	void WriteableBuffer::WriteString(const std::string& value)
	{
		uint32_t length = (uint32_t)(value.length() + 1);
		Write(value.c_str(), length);
	}

	void WriteableBuffer::Write(const char* value, uint32_t& size)
	{
		Write(size);

		buffer.reserve(buffer.size() + size);
		for (int i = 0; i < size; i++)
		{
			buffer.push_back(value[i]);
		}
	}

	template<typename T>
	void WriteableBuffer::Write(T& value)
	{
		char* charArr = (char*)&value;

		for (int i = 0; i < sizeof(value); i++)
			buffer.push_back(charArr[i]);
	}

	template<typename T>
	void WriteableBuffer::Write(T&& value)
	{
		T val = value;
		Write(val);
	}
}
