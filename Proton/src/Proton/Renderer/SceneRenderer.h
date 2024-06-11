#pragma once
#include "Proton\Scene\Scene.h"
#include "EditorCamera.h"
#include "Proton/Model/Material.h"

namespace Proton
{
	class Mesh;

	class SceneRenderer
	{
		friend class Scene;
	public:
		SceneRenderer() = default;
		SceneRenderer(const Ref<Scene> scene, const FramebufferDescription& framebufferDesc)
			:
			m_Scene(scene)
		{
			m_FrameBuffer = Framebuffer::Create(framebufferDesc);

			//TEMP (debug lines)
			m_DebugPixShader = PixelShader::CreateUnique(CoreUtils::CORE_PATH_STR + "Proton\\DebugPS.cso");
			m_DebugVertShader = VertexShader::CreateUnique(CoreUtils::CORE_PATH_STR + "Proton\\DebugVS.cso");
			m_ViewProjBuffer = VertexConstantBuffer::CreateUnique(0, sizeof(DirectX::XMMATRIX), nullptr);

			//TEMP
			DCB::RawLayout layout;
			layout.Add(DCB::Type::Float3, "pos");
			layout.Add(DCB::Type::Float3, "ambient");
			layout.Add(DCB::Type::Float3, "diffuseColor");
			layout.Add(DCB::Type::Float, "diffuseIntensity");
			layout.Add(DCB::Type::Float, "attConst");
			layout.Add(DCB::Type::Float, "attLin");
			layout.Add(DCB::Type::Float, "attQuad");

			lightCBuf = PixelConstantBuffer::CreateUnique(0, DCB::CookedLayout(std::move(layout)));
		}

		//TODO: Replace with camera object 
		virtual void Render(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix);

		void SetScene(const Ref<Scene> scene) { m_Scene = scene; }

		void Resize(uint32_t width, uint32_t height) { m_FrameBuffer->Resize(width, height); }

		void* GetRenderTextureID(int index) { return m_FrameBuffer->GetRenderTextureID(index); }

		Ref<Framebuffer> GetFrameBuffer() { return m_FrameBuffer; }

	protected:
		virtual void SubmitMesh(Entity entity, const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf);

	private:
		void SubmitNode(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection);
		//void DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection);
	protected:
		Ref<Framebuffer> m_FrameBuffer;
		Ref<Scene> m_Scene;

		//TEMP (for debug lines)
		Scope<VertexBuffer> m_DebugVertBuffer;
		Scope<VertexShader> m_DebugVertShader;
		Scope<PixelShader> m_DebugPixShader;
		Scope<VertexConstantBuffer> m_ViewProjBuffer;

	protected:
		//TEMP
		struct PointLightData
		{
			alignas(16) DirectX::XMFLOAT3 pos = {0, 0, 0};
			alignas(16) DirectX::XMFLOAT3 ambient = { 0, 0, 0 };
			alignas(16) DirectX::XMFLOAT3 diffuseColor = { 0, 0, 0 };
			float diffuseIntensity = 0;
			float attConst = 0;
			float attLin = 0;
			float attQuad = 0;
		};

		//TEMP
		Scope<PixelConstantBuffer> lightCBuf;

	};
}