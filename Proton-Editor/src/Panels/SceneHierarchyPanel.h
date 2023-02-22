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

		static void SetContext(const Ref<Scene> scene);

		static void OnImGuiRender();
	private:
		void DrawEntityNode(UUID entityID);
		void DrawChildNode(UUID entityID);
		void DeleteChildNode(UUID entityID);
		void DrawComponents(Entity entity);

		static SceneHierarchyPanel& Get()
		{
			static SceneHierarchyPanel panel;
			return panel;
		}

		static void DragProcedure(Entity entity, int& position);
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;
	};
}