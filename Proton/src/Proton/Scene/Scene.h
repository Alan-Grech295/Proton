#pragma once
#include "entt.hpp"
#include "Proton\Core\TimeStep.h"
#include "Proton\Renderer\Framebuffer.h"

namespace Proton
{
	class Entity;
	struct TransformComponent;
	struct ChildNodeComponent;
	struct MeshComponent;

	class Scene
	{
		friend class Entity;
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		void OnUpdate(TimeStep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		void DrawChildren(TransformComponent& transform, DirectX::FXMMATRIX& accumulatedTransform, ChildNodeComponent& node, MeshComponent& mesh, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection);
	public:
		Ref<Framebuffer> framebuffer;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
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