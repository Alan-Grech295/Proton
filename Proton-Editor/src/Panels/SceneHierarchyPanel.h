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

		void SetContext(const Ref<Scene> scene);

		void OnImGuiRender();

		Entity GetSelectedEntity() const { return m_Selected; }
	private:
		void DrawEntityNode(UUID entityID);
		void DrawChildNode(UUID entityID);
		void DeleteChildNode(UUID entityID);
		void DrawComponents(Entity entity);

		void DragProcedure(Entity entity, int& position);

		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);
	private:
		Ref<Scene> m_Context;
		Entity m_Selected;
	};
}