#include "ptpch.h"
#include "AssetManager.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <Platform\DirectX 11\DirectXTexture.h>
#include "Proton\Model\Model.h"
#include "Proton/Asset Loader/AssetCollection.h"
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
		//ModelCreator::Serialize(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Models/nano_textured/nanosuit.obj");
		
		//ModelCreator::Deserialize(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Models/nano_textured/nanosuit.obj");

		/*Asset& desAsset = AssetSerializer::DeserializeAsset(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Prefabs/asset.asset");

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

		/*RawAsset rawAsset;
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

		//Asset asset(rawAsset);

		//AssetSerializer::SerializeAsset(manager.m_ProjectPath.generic_string() + "/Proton-Editor/assets/Prefabs/asset.asset", asset);//*/

		//159 bytes, 25 elements

		manager.m_AssetImports.clear();

		manager.ScanDirectory(manager.m_ProjectPath.string());

		/*for (auto& path : manager.m_PostImageImports)
		{
			File* file = manager.WriteImageData(path);

			file->WriteFile();

			delete file;
		}*/

		for (auto& path : manager.m_AssetImports)
		{
#define X(ext, type, cls) if(path.extension() == ext) { AssetCollection::Add(path.string(), cls::Serialize(path.string())); continue; }
			ASSET_TYPES
#undef X
		}
	}

	void AssetManager::SetProjectPath(const std::filesystem::path path)
	{
		manager.m_ProjectPath = path;
	}

	Ref<Image> AssetManager::GetImage(const std::string& path)
	{
		return manager.m_ImageAssets[path];
	}

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

	void AssetManager::ScanDirectory(const std::filesystem::path& path)
	{
		for (const auto& entry : fs::directory_iterator(path)) {
			if (entry.is_directory()) 
			{
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
		if (path.extension() == ".asset")
		{
			Ref<Asset> asset = AssetSerializer::DeserializeAsset(path.string());
			AssetType assetType = (AssetType)((uint32_t)(*asset)["AssetType"]);
			std::filesystem::path modelPath = path;
			modelPath = modelPath.replace_extension();
#define X(ext, type, cls) if(type == assetType) { AssetCollection::Add(modelPath.string(), cls::Deserialize(*asset.get(), modelPath.string())); return; }
			ASSET_TYPES
#undef X
			return;
		}

		if (std::filesystem::exists(path.string() + ".asset"))
			return;

#define X(ext, type, cls) if(path.extension() == ext) { manager.m_AssetImports.push_back(path); return; }
		ASSET_TYPES
#undef X
	}

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
		for (uint32_t i = 0; i < size; i++)
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
