#pragma once
#include "Proton\Renderer\Buffer.h"
#include <DirectXMath.h>
#include <memory>
#include <vector>

struct aiNode;
struct aiMesh;

namespace Proton
{
	using RenderCallback = std::function<void(const UINT)>;

	class Mesh
	{
		friend class Node;
		friend class Model;
		friend class Renderer;
	public:
		Mesh() 
			:
			m_Transform(DirectX::XMMatrixIdentity())
		{}
	private:
		DirectX::XMMATRIX GetTransformXM() const { return m_Transform; }
		void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const;
	private:
		DirectX::XMMATRIX m_Transform;
		//Temp (TODO: Add them as "bindables")
		std::unique_ptr<VertexBuffer> m_VertBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<VertexShader> m_VertShader;
		std::unique_ptr<PixelShader> m_PixelShader;
		std::unique_ptr<VertexConstantBuffer> m_TransformCBuf;
		std::unique_ptr<PixelConstantBuffer> m_MaterialCBuf;

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	};

	class Node
	{
		friend class Model;
	public:
		Node(std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX transform);
		void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const;
		//Draw Function
	private:
		void AddChild(std::unique_ptr<Node> pChild);
	private:
		std::vector<std::unique_ptr<Node>> m_ChildPtrs;
		std::vector<Mesh*> m_MeshPtrs;
		DirectX::XMMATRIX m_Transform;
	};

	class Model
	{
	public:
		Model(const std::string& modelPath);
		void Bind(RenderCallback callback, DirectX::FXMMATRIX transform) const;
		//Draw Function
	private:
		static std::unique_ptr<Mesh> ParseMesh(const aiMesh& mesh);
		std::unique_ptr<Node> ParseNode(const aiNode& node);
	private:
		std::unique_ptr<Node> m_Root;
		std::vector<std::unique_ptr<Mesh>> m_MeshPtrs;
	};
}