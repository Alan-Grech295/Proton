#pragma once
#include "entt.hpp"
#include "Proton\Core\TimeStep.h"
#include "Proton\PointLight.h"
#include "Components.h"
#include "Proton\Renderer\Framebuffer.h"

namespace Proton
{
	class Entity;
	class Scene
	{
		friend class Entity;
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		void OnUpdate(TimeStep ts, Ref<PointLight> light);
	private:
		void DrawChildren(TransformComponent& transform, DirectX::FXMMATRIX& accumulatedTransform, ChildNodeComponent& node, MeshComponent& mesh, DirectX::FXMMATRIX& cameraView, DirectX::FXMMATRIX& cameraProjection);
	public:
		Ref<Framebuffer> framebuffer;
	private:
		entt::registry m_Registry;
	};
}