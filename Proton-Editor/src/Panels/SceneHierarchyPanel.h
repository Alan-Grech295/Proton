#pragma once
#include "Proton\Scene\Scene.h"
#include "Proton\Scene\Entity.h"
#include "Proton\Core\Core.h"
#include "Proton\Scene\Components.h"

namespace Proton
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const Ref<Scene>& scene);
		~SceneHierarchyPanel();

		void SetScene(const Ref<Scene>& scene);

		void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawChildNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		Ref<Scene> m_Scene;
		Entity m_Selected;
	};
}