#include "ptpch.h"
#include "Model.h"
#include "Proton\Renderer\Renderer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Proton\Log.h"
#include <unordered_map>
#include <imgui.h>

namespace Proton
{
	//Node
	Node::Node(std::string name, std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX transform)
		:
		m_MeshPtrs(std::move(meshPtrs)),
		m_Transform(transform),
		m_Name(name)
	{
	}

	void Node::Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform) const
	{
		const auto built = m_AppliedTransform *
						   m_Transform * 
						   accumulatedTransform;

		for (const auto pm : m_MeshPtrs)
		{
			pm->Bind(callback, built);
		}

		for (const auto& pc : m_ChildPtrs)
		{
			pc->Bind(callback, built);
		}
	}

	void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept
	{
		m_AppliedTransform = transform;
	}

	void Node::AddChild(std::unique_ptr<Node> pChild)
	{
		assert(pChild);
		m_ChildPtrs.push_back(std::move(pChild));
	}

	void Node::ShowTree(int& nodeIndex, std::optional<int>& selectedIndex, Node*& pSelectedNode) const noexcept
	{
		// nodeIndex serves as the uid for gui tree nodes, incremented throughout recursion
		const int currentNodeIndex = nodeIndex;
		nodeIndex++;
		// build up flags for current node
		const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
			| ((currentNodeIndex == selectedIndex.value_or(-1)) ? ImGuiTreeNodeFlags_Selected : 0)
			| ((m_ChildPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
		// render this node
		const auto expanded = ImGui::TreeNodeEx(
			(void*)(intptr_t)currentNodeIndex, node_flags, m_Name.c_str()
		);
		// processing for selecting node
		if (ImGui::IsItemClicked())
		{
			selectedIndex = currentNodeIndex;
			pSelectedNode = const_cast<Node*>(this);
		}
		// recursive rendering of open node's children
		if (expanded)
		{
			for (const auto& pChild : m_ChildPtrs)
			{
				pChild->ShowTree(nodeIndex, selectedIndex, pSelectedNode);
			}
			ImGui::TreePop();
		}
	}

	//Model
	class ModelWindow // pImpl idiom, only defined in this .cpp
	{
	public:
		void Show(const char* windowName, const Node& root) noexcept
		{
			// window name defaults to "Model"
			windowName = windowName ? windowName : "Model";
			// need an ints to track node indices and selected node
			int nodeIndexTracker = 0;
			if (ImGui::Begin(windowName))
			{
				ImGui::Columns(2, nullptr, true);
				root.ShowTree(nodeIndexTracker, selectedIndex, pSelectedNode);

				ImGui::NextColumn();
				if (pSelectedNode != nullptr)
				{
					auto& transform = transforms[*selectedIndex];
					ImGui::Text("Orientation");
					ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
					ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
					ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
					ImGui::Text("Position");
					ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
					ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
					ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
				}
			}
			ImGui::End();
		}

		DirectX::XMMATRIX GetTransform() const noexcept
		{
			const auto& transform = transforms.at(*selectedIndex);
			return
				DirectX::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
				DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);
		}

		Node* GetSelectedNode() const noexcept
		{
			return pSelectedNode;
		}
	private:
		std::optional<int> selectedIndex;
		Node* pSelectedNode;
		struct TransformParameters
		{
			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = 0.0f;
			float x = 0.0f;
			float y = 0.0f;
			float z = 0.0f;
		};
		std::unordered_map<int, TransformParameters> transforms;
	};

	Model::Model(const std::string& modelPath)
		:
		pWindow(std::make_unique<ModelWindow>())
	{	
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(modelPath.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GenNormals);

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			m_MeshPtrs.push_back(ParseMesh(*pScene->mMeshes[i], pScene->mMaterials));
		}

		m_Root = ParseNode(*pScene->mRootNode);
	}

	void Model::Bind(RenderCallback callback, DirectX::FXMMATRIX transform) const
	{
		if (auto node = pWindow->GetSelectedNode())
		{
			node->SetAppliedTransform(pWindow->GetTransform());
		}

		m_Root->Bind(callback, transform);
	}

	void Model::ShowWindow(const char* windowName) noexcept
	{
		pWindow->Show(windowName, *m_Root);
	}

	Model::~Model() noexcept
	{}

	std::unique_ptr<Mesh> Model::ParseMesh(const aiMesh& mesh, const aiMaterial* const* pMaterials)
	{
		namespace dx = DirectX;
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
			dx::XMFLOAT2 uv;
		};

		std::vector<Vertex> vertices;
		vertices.reserve(mesh.mNumVertices);

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vertices.push_back({
				{ mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z },
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
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

		float shininess = 40.0f;

		if (mesh.mMaterialIndex >= 0)
		{
			using namespace std::string_literals;
			auto& material = *pMaterials[mesh.mMaterialIndex];

			const auto base = "C:\\Dev\\Proton\\Proton\\Models\\nano_textured\\"s;
			aiString texFileName;

			material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName);
			pMesh->m_Texture.reset(Texture2D::Create(base + texFileName.C_Str()));
			pMesh->hasSpecular = false;

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasSpecular = true;
				pMesh->m_Specular.reset(Texture2D::Create(base + texFileName.C_Str(), 1));
			}
			else
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			pMesh->m_Sampler.reset(Sampler::Create());
		}

		pMesh->m_VertBuffer.reset(VertexBuffer::Create(sizeof(Vertex), vertices.data(), (uint32_t)vertices.size()));

		pMesh->m_IndexBuffer.reset(IndexBuffer::Create(indices.data(), (uint32_t)indices.size()));

		if (pMesh->hasSpecular)
		{
			pMesh->m_PixelShader.reset(PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongSpecularPS.cso"));

		}
		else
		{
			pMesh->m_PixelShader.reset(PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongPS.cso"));
		}

		pMesh->m_VertShader.reset(VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongVS.cso"));


		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3},
			{"TEXCOORD", ShaderDataType::Float2}
		};

		pMesh->m_VertBuffer->SetLayout(layout, pMesh->m_VertShader.get());

		struct PSMaterialConstant
		{
			float specularIntensity = 1.0f;
			float specularPower;
			float padding[2] = { 0, 0 };
		} pmc;

		pmc.specularPower = shininess;
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

		auto pNode = std::make_unique<Node>(node.mName.C_Str(), std::move(curMeshPtrs), transform);

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

		if (hasSpecular)
		{
			m_Specular->Bind();
		}
		else
		{
			m_MaterialCBuf->Bind();
		}

		m_Texture->Bind();
		m_Sampler->Bind();

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