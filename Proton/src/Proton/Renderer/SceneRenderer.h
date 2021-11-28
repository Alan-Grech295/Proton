#pragma once
#include "Proton\Scene\Scene.h"

namespace Proton
{
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
			/*m_DebugPixShader = PixelShader::CreateUnique("C:\\Dev\\Proton\\Proton\\DebugPS.cso");
			m_DebugVertShader = VertexShader::CreateUnique("C:\\Dev\\Proton\\Proton\\DebugVS.cso");
			m_ViewProjBuffer = VertexConstantBuffer::CreateUnique(0, sizeof(DirectX::XMMATRIX), nullptr);*/
		}

		//TODO: Replace with camera object 
		void Render(const Camera& camera, DirectX::FXMMATRIX& cameraView);

	private:
		void DrawChildren(Entity entity, DirectX::FXMMATRIX& accumulatedTransform, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection);
	public:
		Ref<Framebuffer> m_FrameBuffer;
	private:
		const Ref<Scene> m_Scene;

		/*TEMP (for debug lines)
		Scope<VertexBuffer> m_DebugVertBuffer;
		Scope<VertexShader> m_DebugVertShader;
		Scope<PixelShader> m_DebugPixShader;
		Scope<VertexConstantBuffer> m_ViewProjBuffer;*/

	private:
		//TEMP
		struct PointLightData
		{
			alignas(16) DirectX::XMFLOAT3 pos;
			alignas(16) DirectX::XMFLOAT3 ambient;
			alignas(16) DirectX::XMFLOAT3 diffuseColor;
			float diffuseIntensity;
			float attConst;
			float attLin;
			float attQuad;
		};

	};
}