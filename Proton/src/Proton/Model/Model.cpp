#include "ptpch.h"
#include "Model.h"
#include "Proton\Renderer\Renderer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Proton\Log.h"

namespace Proton
{
	//Node
	Node::Node(std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX transform)
		:
		m_MeshPtrs(std::move(meshPtrs)),
		m_Transform(transform)
	{
	}

	void Node::Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const
	{
		const auto built = m_Transform * accumulatedTransform;

		for (const auto pm : m_MeshPtrs)
		{
			pm->Bind(callback, built);
		}

		for (const auto& pc : m_ChildPtrs)
		{
			pc->Bind(callback, built);
		}
	}

	void Node::AddChild(std::unique_ptr<Node> pChild)
	{
		assert(pChild);
		m_ChildPtrs.push_back(std::move(pChild));
	}

	//Model
	Model::Model(const std::string& modelPath)
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(modelPath.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices);

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			m_MeshPtrs.push_back(ParseMesh(*pScene->mMeshes[i]));
		}

		m_Root = ParseNode(*pScene->mRootNode);
	}

	void Model::Bind(RenderCallback callback, DirectX::FXMMATRIX transform) const
	{
		m_Root->Bind(callback, transform);
	}

	std::unique_ptr<Mesh> Model::ParseMesh(const aiMesh& mesh)
	{
		namespace dx = DirectX;
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
		};

		std::vector<Vertex> vertices;
		vertices.reserve(mesh.mNumVertices);

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vertices.push_back({
				{ mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z },
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
				});
		}

		PT_CORE_INFO("{0}", mesh.mNumVertices);

		std::vector<unsigned short> indices;
		indices.reserve((UINT)mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		std::unique_ptr<Mesh> pMesh = std::make_unique<Mesh>();

		pMesh->m_VertBuffer.reset(VertexBuffer::Create(sizeof(Vertex), vertices.data(), (uint32_t)vertices.size()));

		pMesh->m_IndexBuffer.reset(IndexBuffer::Create(indices.data(), (uint32_t)indices.size()));

		pMesh->m_VertShader.reset(VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongVS.cso"));

		pMesh->m_PixelShader.reset(PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongPS.cso"));

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3}
		};

		pMesh->m_VertBuffer->SetLayout(layout, pMesh->m_VertShader.get());

		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[3] = { 0, 0, 0 };
		} pmc;

		pMesh->m_MaterialCBuf.reset(PixelConstantBuffer::Create(1, sizeof(pmc), &pmc));

		pMesh->m_TransformCBuf.reset(VertexConstantBuffer::Create(0, sizeof(Mesh::Transforms), new Mesh::Transforms()));

		return pMesh;
	}

	std::unique_ptr<Node> Model::ParseNode(const aiNode& node)
	{
		namespace dx = DirectX;
		const auto transform = dx::XMMatrixTranspose(
			dx::XMLoadFloat4x4(
				reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
			)
		);

		std::vector<Mesh*> curMeshPtrs;
		curMeshPtrs.reserve(node.mNumMeshes);
		for (size_t i = 0; i < node.mNumMeshes; i++)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(m_MeshPtrs.at(meshIdx).get());
		}

		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);

		for (size_t i = 0; i < node.mNumChildren; i++)
		{
			pNode->AddChild(ParseNode(*node.mChildren[i]));
		}

		return pNode;
	}

	void Mesh::Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const
	{
		m_VertBuffer->Bind();
		m_IndexBuffer->Bind();
		m_VertShader->Bind();
		m_PixelShader->Bind();
		m_MaterialCBuf->Bind();

		const auto modelView = accumulatedTransform * Renderer::GetCamera().GetViewMatrix();
		const Transforms tf =
		{
			DirectX::XMMatrixTranspose(modelView),
			DirectX::XMMatrixTranspose(
				modelView *
				Renderer::GetCamera().GetProjectionMatrix()
			)
		};

		m_TransformCBuf->SetData(sizeof(Transforms), &tf);

		m_TransformCBuf->Bind();

		callback(m_IndexBuffer->GetCount());
	}
}