#pragma once
#include "Proton\Renderer\Buffer.h"
#include "Proton\Renderer\Texture.h"
#include "Proton\Renderer\Sampler.h"
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

	class Mesh
	{
		friend class Node;
		friend class Model;
		friend class Renderer;
	public:
		//Mesh(std::vector<std::unique_ptr<Bindable>> binds);
		Mesh(const std::string& meshTag)
			:
			m_Transform(DirectX::XMMatrixIdentity())
		{
			m_TransformCBuf = VertexConstantBuffer::CreateUnique(0, sizeof(Transforms), new Transforms());
			m_TransformCBufPix = PixelConstantBuffer::CreateUnique(2, sizeof(Transforms), new Transforms());
		}
	private:
		DirectX::XMMATRIX GetTransformXM() const { return m_Transform; }
		void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const;
	private:
		DirectX::XMMATRIX m_Transform;

		Ref<VertexBuffer> m_VertBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexShader> m_VertShader;
		Ref<PixelShader> m_PixelShader;
		Ref<class Texture2D> m_Diffuse;
		Ref<class Texture2D> m_Specular;
		Ref<class Texture2D> m_Normal;
		Ref<class Sampler> m_Sampler;

		Scope<PixelConstantBuffer> m_MaterialCBuf;
		Scope<VertexConstantBuffer> m_TransformCBuf;
		Scope<PixelConstantBuffer> m_TransformCBufPix;

		bool hasSpecular = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	};

	class Node
	{
		friend class Model;
		friend class ModelWindow;
	public:
		Node(std::string name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX transform);
		void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const;
		void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;
	private:
		void AddChild(Scope<Node> pChild);
		void ShowTree(int& nodeIndex, std::optional<int>& selectedIndex, Node*& pSelectedNode) const noexcept;
	private:
		std::string m_Name;
		std::vector<Scope<Node>> m_ChildPtrs;
		std::vector<Mesh*> m_MeshPtrs;
		DirectX::XMMATRIX m_Transform;
		DirectX::XMMATRIX m_AppliedTransform = DirectX::XMMatrixIdentity();
	};

	class Model
	{
	public:
		Model(const std::string& modelPath);
		void Bind(RenderCallback callback, DirectX::FXMMATRIX transform) const;
		void ShowWindow(const char* windowName = nullptr) noexcept;
		~Model() noexcept;
	private:
		static Scope<Mesh> ParseMesh(const std::string& basePath, const aiMesh& mesh, const aiMaterial* const* pMaterials);
		Scope<Node> ParseNode(const aiNode& node);
	private:
		Scope<Node> m_Root;
		std::vector<Scope<Mesh>> m_MeshPtrs;
		Scope<class ModelWindow> pWindow;
	};
}