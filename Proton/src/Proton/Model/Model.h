#pragma once
#include "Proton\Renderer\Render Queue\Technique.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <optional>
#include <cassert>
#include <assimp/scene.h>

#include "Bindables/ConstructableBindable.h"
#include "Proton/Asset Loader/AssetCollection.h"
#include "Proton/Asset Loader/AssetSerializer.h"

struct aiNode;
struct aiMesh;
struct aiMaterial;

namespace Proton
{
	//using RenderCallback = std::function<void(const UINT)>;
	class Entity;
	class Scene;

	class Material
	{
	public:
		Material()
		{
			m_Techniques = std::vector<Technique>();
		}

		void AddTechnique(Technique& technique)
		{
			m_Techniques.push_back(technique);
		}

		std::vector<Technique>::iterator begin() { return m_Techniques.begin(); }
		std::vector<Technique>::iterator end() { return m_Techniques.end(); }
	public:
		std::vector<Technique> m_Techniques;

		bool hasSpecular = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;
	};

	class Mesh
	{
		friend class ModelCreator;
		friend class Renderer;
		friend class SceneRenderer;
		friend class AssetManager;
		friend struct MeshData;
	public:
		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	public:
		Mesh(const std::string& name, const std::string& modelPath, Material& material)
			:
			m_Name(name),
			m_ModelPath(modelPath),
			m_Material(material)
		{
			m_TransformCBuf = VertexConstantBuffer::CreateUnique(0, sizeof(Transforms), new Transforms());
			m_TransformCBufPix = PixelConstantBuffer::CreateUnique(2, sizeof(Transforms), new Transforms());
			m_Topology = Topology::Create(TopologyType::TriangleList);
		}

		Mesh() = default;

		Mesh(const Mesh& mesh) = default;
		
		//void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform, DirectX::FXMMATRIX cameraView, DirectX::FXMMATRIX projectionMatrix) const;
	public:
		std::string m_ModelPath;
		std::string m_Name;
	private:
		Ref<Bindable> m_VertBuffer;
		Ref<Bindable> m_IndexBuffer;
		Ref<Bindable> m_Topology;
		Material& m_Material;

		//TODO: Change pointer to proxy
		Ref<VertexConstantBuffer> m_TransformCBuf;
		Ref<PixelConstantBuffer> m_TransformCBufPix;
	};

	class Node
	{
		std::string m_Name;
		std::vector<Node*> m_Children;
		DirectX::XMMATRIX m_Transformation;
		std::vector<Mesh*> m_Meshes;
	};

	class Model
	{
	public:
		Mesh* FindMeshWithName(const std::string& name)
		{
			for (Mesh& mesh : m_Meshes)
			{
				if (mesh.m_Name == name)
					return &mesh;
			}
		}
	public:
		std::vector<Mesh> m_Meshes;
		std::vector<Material> m_Materials;
	};

	struct StepData
	{
	public:
		StepData()
			:
			m_ID(-1)
		{}

		StepData(const std::string& name)
			:
			m_ID(Renderer::GetPassIDFromName(name))
		{}

		StepData(int id)
			:
			m_ID(id)
		{}

		void AddBindable(Ref<ConstructableBindable> bindable)
		{
			m_Bindables.push_back(bindable);
		}

		template<typename T>
		T* GetResource(ConstructableBindable::ResourceType resource) const
		{
			for (Ref<ConstructableBindable> bindable : m_Bindables)
			{
				if (bindable->m_ResourceType == resource)
					return (T*)bindable.get()->GetRef().get();
			}

			return nullptr;
		}

		Step CreateStep()
		{
			Step step = Step(m_ID);

			for (auto bind : m_Bindables)
			{
				step.AddBindable(bind->GetRef());
			}

			return step;
		}
	public:
		std::vector<Ref<ConstructableBindable>> m_Bindables;
		int m_ID;
	};

	struct TechniqueData
	{
	public:
		TechniqueData()
			:
			m_Name("")
		{}
		TechniqueData(const std::string& name)
			:
			m_Name(name)
		{}

		Technique CreateTechnique()
		{
			Technique tech = Technique(m_Name);

			for (StepData& step : m_Steps)
			{
				tech.AddStep(step.CreateStep());
			}

			return tech;
		}
	public:
		std::vector<StepData> m_Steps = std::vector<StepData>();
		std::string m_Name;
	};

	struct MaterialData
	{
	public:
		MaterialData()
		{
			m_Techniques = std::vector<TechniqueData>();
		}
		
		Material CreateMaterial()
		{
			Material mat = Material();

			for (TechniqueData& tech : m_Techniques)
			{
				mat.AddTechnique(tech.CreateTechnique());
			}

			return mat;
		}

	public:
		std::vector<TechniqueData> m_Techniques;

		bool hasSpecular = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;
	};

	struct MeshData
	{
	public:
		MeshData() {}
		void Create(const std::string& name, const std::string& modelPath)
		{
			m_Name = name;
			m_ModelPath = modelPath;
			m_TransformCBuf = CreateRef<UniqueBindable>(ConstructableBindable::ResourceType::VertexConstantBuffer);
			m_TransformCBuf->Initialize<VertexConstantBuffer>(0, sizeof(Mesh::Transforms), new Mesh::Transforms());

			m_TransformCBufPix = CreateRef<UniqueBindable>(ConstructableBindable::ResourceType::PixelConstantBuffer);
			m_TransformCBufPix->Initialize<PixelConstantBuffer>(2, sizeof(Mesh::Transforms), new Mesh::Transforms());
		
			m_Topology = CreateRef<SharedBindable>(ConstructableBindable::ResourceType::Topology, "TriangleList");
			m_Topology->m_Bindable = Topology::Create(TopologyType::TriangleList);
		}

		Mesh CreateMesh(std::vector<Material>& materials)
		{
			Mesh mesh = Mesh(m_Name, m_ModelPath, materials[m_Material]);

			mesh.m_VertBuffer = m_VertBuffer->GetRef();
			mesh.m_IndexBuffer = m_IndexBuffer->GetRef();
			mesh.m_Topology = m_Topology->GetRef();

			//Transform buffers?
			return mesh;
		}
	public:
		std::string m_ModelPath;
		std::string m_Name;

		Ref<SharedBindable> m_VertBuffer;
		Ref<SharedBindable> m_IndexBuffer;
		Ref<SharedBindable> m_Topology;
		uint32_t m_Material;

		Ref<UniqueBindable> m_TransformCBuf;
		Ref<UniqueBindable> m_TransformCBufPix;
	};

	struct NodeData
	{
		std::string m_Name;
		std::vector<uint32_t> m_Children;
		DirectX::XMMATRIX m_Transformation;
		std::vector<uint32_t> m_Meshes;
	};

	struct ModelData
	{
	public:
		ModelData() = default;
		ModelData(const ModelData& copy)
			:
			m_Meshes(copy.m_Meshes),
			m_Materials(copy.m_Materials),
			m_Nodes(copy.m_Nodes)
		{
		}

		std::vector<MeshData> m_Meshes;
		std::vector<MaterialData> m_Materials;
		std::vector<NodeData> m_Nodes;
	};

	class ModelCreator
	{
	public:
		static Ref<ModelData> Serialize(const std::string& path);
		static Ref<ModelData> Deserialize(Asset& modelAsset, const std::string& path);
		static Entity CreateModelEntity(const std::string& path, Scene& activeScene);
		static Ref<Model> GetModelFromData(ModelData& modelData);
	private:
		static TypeElement SerializeMesh(MeshData* meshData, RawAsset& asset, const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiScene* scene, const MaterialData* materials);
		static void SerializeMaterial(MaterialData* matData, RawAsset& asset, const std::string& basePath, uint32_t index, const aiMaterial& aiMat);
		static void SerializeNode(NodeData*& nodeData, aiNode* aiNode, RawAsset& asset, MeshData* meshes, uint32_t& index);
		static uint32_t CountNodes(aiNode* root);

		static void DeserializeMeshes(Asset& asset, const std::string& modelPath, MeshData* meshData, const MaterialData* materials);
		static void DeserializeMaterials(Asset& asset, MaterialData* materialData);
		static void DeserializeNodes(Asset& asset, NodeData* nodeData, MeshData* meshData);
	};

	//TEMP
	class ModelCollection
	{
	public:
		static void Add(UUID uuid, Ref<Model> model) { m_Models[uuid] = model; }
		static Ref<Model> Get(UUID uuid) { return m_Models.at(uuid); }
		static Ref<Model> GetOrCreate(UUID uuid, const std::string& path)
		{
			if (m_Models.find(uuid) != m_Models.end())
				return m_Models.at(uuid);

			Ref<Model> model = ModelCreator::GetModelFromData(*AssetCollection::Get<ModelData>(path));
			m_Models[uuid] = model;
			return model;
		}
	private:
		static std::unordered_map<UUID, Ref<Model>> m_Models;
	};
}