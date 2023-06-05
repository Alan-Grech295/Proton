#pragma once
#include <unordered_map>
#include <filesystem>
#include <vector>

namespace Proton
{
	class Entity;

	struct path_hash
	{
		std::size_t operator() (const std::filesystem::path& path) const
		{
			std::hash<std::string> hasher;
			return hasher(path.string());
		}
	};

	class File
	{
	public:
		File(std::filesystem::path path, uint64_t fileSize);
		~File();

		template<typename T>
		void Write(T& value);

		template<typename T>
		void Write(T&& value);

		void WriteString(const std::string& value);
		void Write(void* value, uint32_t& size, bool writeSize = true);

		template<typename T>
		T Read();

		template<typename T>
		T* ReadArray(uint32_t& arraySize);

		template<typename T>
		T* GetEndArrayPtr();

		std::string ReadString();

		std::filesystem::path GetPath() { return filePath; }

		void WriteFile();
		void ReadFile();

		void ShowSize();

		bool ReachedEnd() { return pointer >= size - 1; }
	private:
		char* buffer;
		uint64_t pointer;
		uint64_t size;
		std::filesystem::path filePath;
	};

	class WriteableBuffer
	{
	public:
		WriteableBuffer();
		~WriteableBuffer();

		template<typename T>
		void Write(T& value);

		template<typename T>
		void Write(T&& value);

		void WriteString(const std::string& value);
		void Write(const char* value, uint32_t& size);

		template<typename T>
		T Read();

		template<typename T>
		T* ReadArray(uint32_t& arraySize);

		std::string ReadString();

		uint64_t size() { return buffer.size(); }
		char* GetBuffer() { return buffer.data(); }
	private:
		std::vector<char> buffer;
	};

	struct Image
	{
		uint32_t width, height;
		uint64_t pixelSize;
		bool isOpaque;
		uint8_t* pixels;

		Image() = default;
	};

	enum class AssetType { Model, Image };
#define ASSET_TYPES X(".obj", Model, ModelCreator)\
					X(".gltf", Model, ModelCreator)\
					X(".blend", Model, ModelCreator)\


	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();

		static void ScanProject();
		static void SetProjectPath(const std::filesystem::path path);
		static Ref<Image> GetImage(const std::string& path);

		//static Ref<Model> GetModel(const std::string& path);

		//static Ref<Prefab> GetPrefab(const std::string& path);

		static void CreatePrefab(Entity& parentEntity, std::string savePath);
	private:
		void ScanDirectory(const std::filesystem::path& path);
		void HandleFile(const std::filesystem::path& path);
		//File* WriteModelData(std::filesystem::path& modelPath);
		File* WriteImageData(const std::filesystem::path& imagePath);

		//void WriteNode(WriteableBuffer* buffer, std::vector<std::string>& modelPaths, uint32_t parentIndex, uint32_t& nodeIndex, Entity& entity);

		//Ref<Model> ImportModelAsset(File& file, const std::string& basePath);
		Ref<Image> ImportImageAsset(File& file);
		//Ref<Prefab> ImportPrefab(File& file);

		//Ref<Prefab> CreatePrefabFromEntity(std::string prefabPath, Entity& parentEntity);
		//PrefabNode* CreateChildPrefab(std::string prefabPath, Entity& childEntity);

		//void NodeTreeSizeCheck(aiNode& node, uint32_t& bufferSize, uint32_t& numChildren);
		//void AddNodeTree(aiNode& node, uint32_t& nodeIndex, std::vector<uint32_t>& tree);
		//Node* CreateNodeTree(aiNode& node, Mesh** meshes);
		//Node* ReadNodeTree(const uint32_t* data, Mesh** meshes, uint32_t nodeIndex);

		template<typename T>
		bool Contains(std::vector<T>& vector, T& obj);

		template<typename T>
		uint32_t GetOrAdd(std::vector<T>& vector, T& obj);
	private:
		std::filesystem::path m_ProjectPath;

		std::unordered_map<std::filesystem::path, Ref<Image>, path_hash> m_ImageAssets;
		//std::unordered_map<std::filesystem::path, Ref<Model>, path_hash> m_ModelAssets;
		//std::unordered_map<std::filesystem::path, Ref<Prefab>, path_hash> m_Prefabs;

		std::vector<std::filesystem::path> m_AssetImports;
		//std::vector<std::pair<std::filesystem::path, char*>> m_PostModelReads;

		std::vector<std::filesystem::path> m_PostImageImports;

		std::vector<std::filesystem::path> m_PostPrefabReads;

		uint32_t m_NextImageID = 1;
		uint32_t m_NextModelID = 1;

		static AssetManager manager;
	};

	template<typename T>
	inline bool AssetManager::Contains(std::vector<T>& vector, T& obj)
	{
		for (T& v : vector)
		{
			if (v == obj)
				return true;
		}

		return false;
	}

	template<typename T>
	inline uint32_t AssetManager::GetOrAdd(std::vector<T>& vector, T& obj)
	{
		if (Contains(vector, obj))
		{
			for (int i = 0; i < vector.size(); i++)
				if (vector[i] == obj) return i;
		}
		else
		{
			vector.push_back(obj);
			return vector.size() - 1;
		}
	}
}