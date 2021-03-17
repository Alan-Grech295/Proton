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
	class Entity;
	class Scene;

	class Mesh
	{
		friend class Model;
		friend class Renderer;
	public:
		Mesh(const std::string& meshTag)
			:
			m_Transform(DirectX::XMMatrixIdentity())
		{
			m_TransformCBuf = VertexConstantBuffer::CreateUnique(0, sizeof(Transforms), new Transforms());
			m_TransformCBufPix = PixelConstantBuffer::CreateUnique(2, sizeof(Transforms), new Transforms());
		}
		
		void Bind(RenderCallback callback, DirectX::FXMMATRIX accumulatedTransform, DirectX::FXMMATRIX cameraView, DirectX::FXMMATRIX projectionMatrix) const;
	private:
		DirectX::XMMATRIX GetTransformXM() const { return m_Transform; }
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

	class Model
	{
	public:
		static Entity CreateModelEntity(const std::string& path, Scene* activeScene);
	private:
		static Entity CreateChild(const aiNode& node, Entity parent, std::vector<Mesh*>& meshPtrs, Scene* activeScene);
		static Mesh* ParseMesh(const std::string& basePath, const aiMesh& mesh, const aiMaterial* const* pMaterials);
	};
}