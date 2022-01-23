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
		Mesh(const std::string& meshTag, const std::string& name, const std::string& modelPath)
			:
			m_Transform(DirectX::XMMatrixIdentity()),
			m_Name(name),
			m_ModelPath(modelPath)
		{
			m_TransformCBuf = VertexConstantBuffer::CreateUnique(0, sizeof(Mesh::Transforms), new Mesh::Transforms());
			m_TransformCBufPix = PixelConstantBuffer::CreateUnique(2, sizeof(Mesh::Transforms), new Mesh::Transforms());
		}

		Mesh() = default;

		Mesh(const Mesh& mesh) = default;
		
		//void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform, DirectX::FXMMATRIX cameraView, DirectX::FXMMATRIX projectionMatrix) const;
	private:
		DirectX::XMMATRIX GetTransformXM() const { return m_Transform; }
	public:
		std::string m_ModelPath;
		std::string m_Name;
	private:
		DirectX::XMMATRIX m_Transform;

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

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	};

	struct Node
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
	};

	class ModelCreator
	{
	public:
		static Entity CreateModelEntity(const std::string& path, Scene* activeScene);
		static Entity CreatePrefabEntity(const std::string& path, Scene* activeScene);
		static Mesh* ParseMesh(const std::string& basePath, const std::string& modelPath, const aiMesh& mesh, const aiMaterial* const* pMaterials);
	private:
		static Entity CreateChild(const Node& node, Entity parent, Entity root, Scene* activeScene);
		static Entity CreatePrefabChild(const PrefabNode& node, Entity parent, Entity root, Scene* activeScene);
	};
}