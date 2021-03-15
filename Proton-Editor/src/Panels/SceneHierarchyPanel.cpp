#include "SceneHierarchyPanel.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace Proton
{
	SceneHierarchyPanel::SceneHierarchyPanel()
	{

	}

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& scene)
	{

	}

	SceneHierarchyPanel::~SceneHierarchyPanel()
	{
	}

	void SceneHierarchyPanel::SetScene(const Ref<Scene>& scene)
	{
		m_Scene = scene;
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		auto& childGroup = m_Scene->m_Registry.group<TransformComponent>(entt::exclude<ChildNodeComponent>);

		for (auto entityID : childGroup)
		{
			Entity entity(entityID, m_Scene.get());
			DrawEntityNode(entity);
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (m_Selected)
		{
			DrawComponents(m_Selected);
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().tag;

		bool isLeaf = true;

		std::vector<Entity>* children = nullptr;

		if (entity.HasComponent<ParentNodeComponent>())
		{
			auto& node = entity.GetComponent<ParentNodeComponent>();
			isLeaf = node.numChildren == 0;
		}

		ImGuiTreeNodeFlags flags = (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) | 
									ImGuiTreeNodeFlags_OpenOnArrow |
									ImGuiTreeNodeFlags_OpenOnDoubleClick |
									(isLeaf ? ImGuiTreeNodeFlags_Leaf : 0);

		bool opened =  ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		if (opened)
		{
			if (entity.HasComponent<ParentNodeComponent>())
			{
				ParentNodeComponent& parentNode = entity.GetComponent<ParentNodeComponent>();

				for (Entity e : parentNode.childNodes)
				{
					DrawChildNode(e);
				}
			}
			ImGui::TreePop();
		}
	}

	void SceneHierarchyPanel::DrawChildNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().tag;

		auto& node = entity.GetComponent<ChildNodeComponent>();

		bool isLeaf = node.numChildren == 0;

		ImGuiTreeNodeFlags flags = (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
									ImGuiTreeNodeFlags_OpenOnArrow |
									ImGuiTreeNodeFlags_OpenOnDoubleClick |
									(isLeaf ? ImGuiTreeNodeFlags_Leaf : 0);

		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		if (opened)
		{
			for (Entity& e : node.childNodes)
			{
				DrawChildNode(e);
			}
			ImGui::TreePop();
		}
	}

	static void DrawFloat3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3, lineHeight };

		//X
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Z
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;

		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, tag.c_str());

			if (ImGui::InputText("Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}
		}

		if (entity.HasComponent<TransformComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
			{
				auto& transform = entity.GetComponent<TransformComponent>();

				static const float degreeToRad = 0.01745329f;
				static const float radToDegree = 57.2957795f;

				DrawFloat3Control("Position", transform.position);

				DirectX::XMFLOAT3 eulerRotation = { transform.rotation.x * radToDegree,
													transform.rotation.y * radToDegree,
													transform.rotation.z * radToDegree };

				DrawFloat3Control("Rotation", eulerRotation);

				transform.rotation = { eulerRotation.x * degreeToRad,
									   eulerRotation.y * degreeToRad,
									   eulerRotation.z * degreeToRad };

				DrawFloat3Control("Scale", transform.scale, 1.0f);				

				ImGui::TreePop();
			}
		}

		if (entity.HasComponent<CameraComponent>())
		{
			if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
			{
				auto& cameraComponent = entity.GetComponent<CameraComponent>();
				auto& camera = cameraComponent.camera;

				const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
				const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
				if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
				{
					for (int i = 0; i < 2; i++)
					{
						bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];

						if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
						{
							currentProjectionTypeString = projectionTypeStrings[i];
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float vertFOV = camera.GetPerspectiveVerticalFOV();
					if (ImGui::DragFloat("Vertical FOV", &vertFOV, 1.0f, 1.0f, 270.f))
						camera.SetPerspectiveVerticalFOV(vertFOV);

					float persNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Clip", &persNear, 1.0f, 0.01f, 10000.0f))
						camera.SetPerspectiveNearClip(persNear);

					float persFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Clip", &persFar, 1.0f, 0.01f, 50000.0f))
						camera.SetPerspectiveFarClip(persFar);
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize, 1.0f, 0.01f))
						camera.SetOrthographicSize(orthoSize);

					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Clip", &orthoNear, 1.0f, 0.01f, 10000.0f))
						camera.SetOrthographicNearClip(orthoNear);

					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Clip", &orthoFar, 1.0f, 0.01f, 50000.0f))
						camera.SetOrthographicFarClip(orthoFar);
				}

				ImGui::TreePop();
			}
		}
	}
}
