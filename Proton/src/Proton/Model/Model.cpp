#include "ptpch.h"
#include "Model.h"
#include "Proton\Renderer\Renderer.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Proton\Log.h"
#include <unordered_map>
#include <imgui.h>
#include <filesystem>

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

	void Node::AddChild(Scope<Node> pChild)
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
			aiProcess_GenNormals |
			aiProcess_CalcTangentSpace);

		std::string basePath = std::filesystem::path(modelPath).remove_filename().string();

		for (size_t i = 0; i < pScene->mNumMeshes; i++)
		{
			m_MeshPtrs.push_back(ParseMesh(basePath, *pScene->mMeshes[i], pScene->mMaterials));
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

	Scope<Mesh> Model::ParseMesh(const std::string& basePath, const aiMesh& mesh, const aiMaterial* const* pMaterials)
	{
		namespace dx = DirectX;
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
			dx::XMFLOAT3 tangent;
			dx::XMFLOAT3 bitangent;
			dx::XMFLOAT2 uv;
		};

		std::vector<Vertex> vertices;
		vertices.reserve(mesh.mNumVertices);

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vertices.push_back({
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
				});
		}

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
		using namespace std::string_literals;

		auto meshTag = basePath + "%" + mesh.mName.C_Str();

		Scope<Mesh> pMesh = std::make_unique<Mesh>(meshTag);

		float shininess = 40.0f;
		bool hasAlphaGloss = false;
		dx::XMFLOAT4 diffuseColor = { 1.0f,0.0f,1.0f,1.0f };
		dx::XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };

		if (mesh.mMaterialIndex >= 0)
		{
			auto& material = *pMaterials[mesh.mMaterialIndex];

			aiString texFileName;

			if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasDiffuseMap = true;
				pMesh->m_Diffuse = Texture2D::Create(basePath + texFileName.C_Str());
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));
			}

			if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasSpecular = true;
				pMesh->m_Specular = Texture2D::Create(basePath + texFileName.C_Str(), 1);
				hasAlphaGloss = pMesh->m_Specular->HasAlpha();
			}
			else
			{
				material.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D&>(specularColor));
			}

			if (!hasAlphaGloss)
			{
				material.Get(AI_MATKEY_SHININESS, shininess);
			}

			if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
			{
				pMesh->hasNormalMap = true;
				pMesh->m_Normal = Texture2D::Create(basePath + texFileName.C_Str(), 2);
				PT_CORE_TRACE(basePath + texFileName.C_Str());
			}

			if(pMesh->hasSpecular || pMesh->hasNormalMap || pMesh->hasDiffuseMap)
				pMesh->m_Sampler = Sampler::Create(meshTag);
		}

		pMesh->m_IndexBuffer = IndexBuffer::Create(meshTag, indices.data(), (uint32_t)indices.size());

		Ref<VertexShader> vs;

		if (pMesh->hasSpecular && !pMesh->hasNormalMap)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongSpecularPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (pMesh->hasNormalMap && !pMesh->hasSpecular)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (pMesh->hasNormalMap && pMesh->hasSpecular)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapSpecPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongNormalMapVS.cso");
		}
		else if (!pMesh->hasNormalMap && !pMesh->hasSpecular && pMesh->hasDiffuseMap)
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongPS.cso");
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongVS.cso");
		}
		else
		{
			pMesh->m_PixelShader = PixelShader::Create("C:\\Dev\\Proton\\Proton\\PhongNoTexPS.cso");
			PT_CORE_TRACE(mesh.mName.C_Str());
			vs = VertexShader::Create("C:\\Dev\\Proton\\Proton\\PhongVS.cso");
		}
		
		pMesh->m_VertShader = vs;

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3},
			{"TANGENT", ShaderDataType::Float3},
			{"BITANGENT", ShaderDataType::Float3},
			{"TEXCOORD", ShaderDataType::Float2}
		};

		Ref<VertexBuffer> vertBuffer = VertexBuffer::Create(meshTag, sizeof(Vertex), vertices.data(), (uint32_t)vertices.size());

		vertBuffer->SetLayout(layout, &*vs);

		pMesh->m_VertBuffer = vertBuffer;

		if (!pMesh->hasDiffuseMap && !pMesh->hasSpecular && !pMesh->hasNormalMap)
		{
			struct PSMaterialConstantNoTex
			{
				dx::XMFLOAT4 materialColor;
				dx::XMFLOAT4 specularColor;
				float specularPower;
				float padding[3];
			} pmc;

			pmc.specularPower = shininess;
			pmc.specularColor = specularColor;
			pmc.materialColor = diffuseColor;
			PT_CORE_INFO(mesh.mName.C_Str());
			pMesh->m_MaterialCBuf = PixelConstantBuffer::Create(meshTag, 1, sizeof(pmc), &pmc);
		}
		else
		{
			struct PSMaterialConstant
			{
				float specularIntensity = 1.0f;
				float specularPower;
				BOOL hasAlphaGloss;
				float padding;
			} pmc;

			pmc.specularPower = shininess;
			pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
			pmc.hasAlphaGloss = hasAlphaGloss ? TRUE : FALSE;
			pMesh->m_MaterialCBuf = PixelConstantBuffer::Create(meshTag, 1, sizeof(pmc), &pmc);
		}

		return pMesh;
	}

	Scope<Node> Model::ParseNode(const aiNode& node)
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

		m_VertBuffer->Bind();
		m_IndexBuffer->Bind();
		m_VertShader->Bind();
		m_PixelShader->Bind();
		m_TransformCBuf->Bind();
		m_MaterialCBuf->Bind();

		if(hasDiffuseMap)
			m_Diffuse->Bind();

		if(hasSpecular)
			m_Specular->Bind();

		if (hasNormalMap)
		{
			m_Normal->Bind();
			m_TransformCBufPix->SetData(sizeof(Transforms), &tf);
			m_TransformCBufPix->Bind();
		}

		if (hasNormalMap || hasSpecular || hasDiffuseMap)
		{
			m_Sampler->Bind();
		}

		callback(m_IndexBuffer->GetCount());
	}

	/*void Mesh::AddBind(std::unique_ptr<Bindable> bind)
	{
		if (typeid(*bind) == typeid(IndexBuffer))
		{
			assert("Binding multiple index buffers not allowed" && m_IndexBuffer == nullptr);
			m_IndexBuffer = &static_cast<IndexBuffer&>(*bind);
		}

		m_Binds.push_back(bind);
	}*/
}