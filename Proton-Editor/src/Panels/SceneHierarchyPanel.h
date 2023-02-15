#pragma once
#include "Proton\Scene\Scene.h"
#include "Proton\Scene\Entity.h"
#include "Proton\Core\Core.h"

namespace Proton
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
		SceneHierarchyPanel(const Ref<Scene>& scene);
		~SceneHierarchyPanel();

		static void SetScene(const Ref<Scene> scene);

		static void OnImGuiRender();
	private:
		void DrawEntityNode(Entity entity);
		void DrawChildNode(Entity entity);
		void DeleteChildNode(Entity entity);
		void DrawComponents(Entity entity);

		static SceneHierarchyPanel& Get()
		{
			static SceneHierarchyPanel panel;
			return panel;
		}
	private:
		Ref<Scene> m_Scene;
		Entity m_Selected;
	};
}