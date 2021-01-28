#include "ptpch.h"
#include "AssimpTest.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Platform\Windows\WindowsGraphics.h"

namespace Proton
{
	AssimpTest::AssimpTest(std::mt19937& rng, std::uniform_real_distribution<float>& adist, std::uniform_real_distribution<float>& ddist, std::uniform_real_distribution<float>& odist, std::uniform_real_distribution<float>& rdist, DirectX::XMFLOAT3 material)
		:
		r(rdist(rng)),
		droll(ddist(rng)),
		dpitch(ddist(rng)),
		dyaw(ddist(rng)),
		dphi(odist(rng)),
		dtheta(odist(rng)),
		dchi(odist(rng)),
		chi(adist(rng)),
		theta(adist(rng)),
		phi(adist(rng))
	{
		namespace dx = DirectX;

		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
		};

		Assimp::Importer imp;
		const auto pModel = imp.ReadFile("C:\\Dev\\Proton\\Proton\\Models\\suzanne.obj",
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices
		);
		const auto pMesh = pModel->mMeshes[0];

		std::vector<Vertex> vertices;
		vertices.reserve(pMesh->mNumVertices);
		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			vertices.push_back({
				{ pMesh->mVertices[i].x,pMesh->mVertices[i].y,pMesh->mVertices[i].z },
				*reinterpret_cast<dx::XMFLOAT3*>(&pMesh->mNormals[i])
				});
		}

		std::vector<unsigned short> indices;
		indices.reserve(pMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
		{
			const auto& face = pMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		m_VertBuffer.reset(VertexBuffer::Create(sizeof(Vertex), &vertices[0], vertices.size()));

		m_IndexBuffer.reset(IndexBuffer::Create(&indices[0], indices.size()));

		m_VertShader.reset(VertexShader::Create(WindowsGraphics::GetShaderPath("PhongVS.cso")));

		m_PixelShader.reset(PixelShader::Create(WindowsGraphics::GetShaderPath("PhongPS.cso")));

		BufferLayout layout = {
			{"POSITION", ShaderDataType::Float3},
			{"NORMAL", ShaderDataType::Float3}
		};

		m_VertBuffer->SetLayout(layout, m_VertShader.get());

		pmc.color = material;
		m_MaterialCBuf.reset(PixelConstantBuffer::Create(0, sizeof(pmc), &pmc));

		m_TransformCBuf.reset(VertexConstantBuffer::Create());
	}
}
