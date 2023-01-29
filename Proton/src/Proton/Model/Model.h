#pragma once
#include "Proton\Renderer\Bindables\Buffer.h"
#include "Proton\Renderer\Bindables\Texture.h"
#include "Proton\Renderer\Bindables\Sampler.h"
#include "Proton\Renderer\Bindables\Topology.h"
#include "Proton\Renderer\Render Queue\Technique.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <optional>
#include <cassert>
#include "Proton/Asset Loader/AssetSerializer.h"
#include <assimp/scene.h>
#include "Bindables/ConstructableBindable.h"

struct aiNode;
struct aiMesh;
struct aiMaterial;

namespace Proton
{
	using RenderCallback = std::function<void(const UINT)>;
	class Entity;
	class Scene;

	class Mesh
	{
		friend class ModelCreator;
		friend class Renderer;
		friend class SceneRenderer;
		friend class AssetManager;
	public:
		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	public:
		Mesh(const std::string& meshTag, const std::string& name, const std::string& modelPath)
			:
			m_Name(name),
			m_ModelPath(modelPath)
		{
			m_TransformCBuf = VertexConstantBuffer::CreateUnique(0, sizeof(Transforms), new Transforms());
			m_TransformCBufPix = PixelConstantBuffer::CreateUnique(2, sizeof(Transforms), new Transforms());
		}

		Mesh() = default;

		Mesh(const Mesh& mesh) = default;
		
		//void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform, DirectX::FXMMATRIX cameraView, DirectX::FXMMATRIX projectionMatrix) const;
	public:
		std::string m_ModelPath;
		std::string m_Name;
	private:
		Ref<VertexBuffer> m_VertBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<Topology> m_Topology = Topology::Create(TopologyType::TriangleList);
		std::vector<Technique> m_Techniques;

		//TODO: Change pointer to proxy
		Ref<VertexConstantBuffer> m_TransformCBuf;
		Ref<PixelConstantBuffer> m_TransformCBufPix;

		//TEMP
		/*Ref<VertexShader> m_VertShader;
		Ref<PixelShader> m_PixelShader;
		Ref<class Texture2D> m_Diffuse;
		Ref<class Texture2D> m_Specular;
		Ref<class Texture2D> m_Normal;
		Ref<class Sampler> m_Sampler;

		Scope<PixelConstantBuffer> m_MaterialCBuf;
		Scope<VertexConstantBuffer> m_TransformCBuf;
		Scope<PixelConstantBuffer> m_TransformCBufPix;*/

		bool hasSpecular = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;
	};

	struct StepData
	{
	public:
		StepData(const std::string& name)
			:
			m_ID(Renderer::GetPassIDFromName(name))
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
		void Create(const std::string& meshTag, const std::string& name, const std::string& modelPath)
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
	public:
		std::string m_ModelPath;
		std::string m_Name;

		Ref<SharedBindable> m_VertBuffer;
		Ref<SharedBindable> m_IndexBuffer;
		Ref<SharedBindable> m_Topology;
		uint32_t m_MatIndex;

		Ref<UniqueBindable> m_TransformCBuf;
		Ref<UniqueBindable> m_TransformCBufPix;
	};

	struct ModelData
	{
		std::vector<MeshData> m_Meshes;
		std::vector<MaterialData> m_Materials;
	};

	/*struct Node
	{
		Node** childNodes;
		uint32_t numChildren;
		DirectX::XMMATRIX transformation;
		Mesh** meshes;
		uint32_t numMeshes;
		std::string name;

		Node() = default;
	};

	struct PrefabNode
	{
		PrefabNode** childNodes;
		uint32_t numChildren;
		DirectX::XMMATRIX transformation;
		Mesh** meshes;
		uint32_t numMeshes;
		std::string name;

		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 scale;

		PrefabNode() = default;
	};

	struct Model
	{
		Node* rootNode;

		Model() = default;
	};

	struct Prefab
	{
		PrefabNode* rootNode;

		Prefab() = default;
	};*/



	class ModelCreator
	{
	public:
		static void ParseModel(const std::string& path);
		static void DeserializeModel(const std::string& path);
	private:
		static TypeElement ParseMesh(MeshData* meshData, RawAsset& asset, const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiScene* scene, const std::vector<MaterialData>& materials);
		static void ParseMaterial(MaterialData* matData, RawAsset& asset, const std::string& basePath, uint32_t index, const aiMaterial& aiMat);
		static void ParseNode(aiNode* node, RawAsset& asset, uint32_t& index);
	};
}