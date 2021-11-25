#include "SceneHierarchyPanel.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "Proton\Core\Input.h"
#include "Proton\Core\KeyCodes.h"
#include "Proton\Scene\AssetManager.h"
#include "AssetViewerPanel.h"

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

	void SceneHierarchyPanel::SetScene(const Ref<Scene> scene)
	{
		Get().m_Scene = scene;
		Get().m_Selected = {};
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		if (!Get().m_Scene)
		{
			ImGui::End();
			ImGui::Begin("Inspector");
			ImGui::End();
			return;
		}

		auto& childView = Get().m_Scene->m_Registry.view<RootNodeTag>();

		for (auto entityID : childView)
		{
			Entity entity(entityID, Get().m_Scene.get());
			Get().DrawEntityNode(entity);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			Get().m_Selected = {};

		//Right-Click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				Get().m_Scene->CreateEntity("New Entity");

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (Get().m_Selected)
		{
			Get().DrawComponents(Get().m_Selected);

			ImGui::Separator();
			ImGui::Text("");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.0f - 55);

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent"))
			{
				if (ImGui::MenuItem("Camera"))
				{
					CameraComponent& camera = Get().m_Selected.AddComponent<CameraComponent>();
					camera.camera.SetViewportSize(Get().m_Scene->GetViewportWidth(), Get().m_Scene->GetViewportHeight());
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Light"))
				{
					Get().m_Selected.AddComponent<LightComponent>();
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	static void DragProcedure(Entity entity, int& position)
	{
		position = -1;

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::GetDragDropPayload() && ImGui::GetDragDropPayload()->IsDataType("SceneHierarchyObject"))
		{
			Entity payload = *(const Entity*)ImGui::GetDragDropPayload()->Data;
			if (payload == entity && !ImGui::IsDragDropPayloadBeingAccepted())
			{
				entity.SetParent(nullptr);
			}
		}

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip | ImGuiDragDropFlags_SourceNoHoldToOpenOthers | ImGuiDragDropFlags_SourceNoDisableHover))
		{
			ImGui::SetDragDropPayload("SceneHierarchyObject", &entity, sizeof(Entity));
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			float percent = (ImGui::GetMousePos().y - ImGui::GetItemRectMin().y) / ImGui::GetItemRectSize().y;

			ImGuiDragDropFlags targetFlags = 0;
			targetFlags |= (percent < 0.25f || percent > 0.75f ? ImGuiDragDropFlags_AcceptNoDrawDefaultRect : 0);

			if (percent < 0.25f)
				position = 1;
			else if (percent > 0.75f)
				position = 0;

			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SceneHierarchyObject", targetFlags))
			{
				Entity draggedEntity = *(const Entity*)payload->Data;

				NodeComponent targetNode = entity.GetComponent<NodeComponent>();

				if (percent < 0.25f)
				{
					position = 1;
					if (entity.HasComponent<RootNodeTag>())
					{
						draggedEntity.SetParent();
					}
					else
					{
						NodeComponent entityParentNode = targetNode.m_ParentEntity.GetComponent<NodeComponent>();
						int pos = 0;
						for (int i = 0; i < entityParentNode.m_ChildNodes.size(); i++)
						{
							if (entityParentNode.m_ChildNodes[i] == entity)
							{
								pos = i;
								break;
							}
						}

						draggedEntity.SetParent(&targetNode.m_ParentEntity, pos);
					}
				}
				else if (percent > 0.75f)
				{
					position = 0;
					if (entity.HasComponent<RootNodeTag>())
					{
						draggedEntity.SetParent(&entity, 0);
					}
					else
					{
						NodeComponent entityParentNode = targetNode.m_ParentEntity.GetComponent<NodeComponent>();
						int pos = 0;
						for (int i = 0; i < entityParentNode.m_ChildNodes.size(); i++)
						{
							if (entityParentNode.m_ChildNodes[i] == entity)
							{
								pos = i;
								break;
							}
						}

						draggedEntity.SetParent(&targetNode.m_ParentEntity, pos + 1);
					}
				}
				else
				{
					draggedEntity.SetParent(&entity);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().tag;
		auto& node = entity.GetComponent<NodeComponent>();
		bool isLeaf = node.m_ChildNodes.size() == 0;

		ImGuiTreeNodeFlags flags = (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			(isLeaf ? ImGuiTreeNodeFlags_Leaf : 0) |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		ImVec2 cursorPos = ImGui::GetCursorPos();

		bool opened = ImGui::TreeNodeEx((void*)((uint32_t)(entity) + 1), flags, tag.c_str());

		int position = -1;
		DragProcedure(entity, position);

		ImVec2 cursorPosNow = ImGui::GetCursorPos();

		if (position == 1)
			ImGui::SetCursorPos(cursorPos);

		if (position != -1)
		{
			ImGui::PushStyleColor(ImGuiCol_Separator, { 1, 1, 0, 1 });
			ImGui::PushItemWidth(ImGui::GetItemRectSize().x);

			ImGui::Separator();

			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::SetCursorPos(cursorPosNow);
		}

		ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 1 });

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		GImGui->CurrentWindow->DC.LastItemId = ((uint32_t)(entity) + 1);

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			if (entity.HasComponent<RootNodeTag>() && ImGui::MenuItem("Create Prefab"))
			{
				std::string tag = entity.GetComponent<TagComponent>().tag;

				uint32_t stringLen = 0;

				for (int i = tag.length() - 1; i >= 0; i--)
				{
					if (tag.at(i) != ' ')
					{
						stringLen = i;
						break;
					}
				}

				AssetManager::CreatePrefab(entity, AssetViewerPanel::GetSelectedPath().string() + "\\" + tag.substr(0, stringLen));
			}

			ImGui::EndPopup();
		}

		if (m_Selected == entity && ImGui::IsWindowHovered() && Input::IsKeyReleased(Key::Delete))
		{
			entityDeleted = true;
		}

		if (opened)
		{
			for (Entity e : node.m_ChildNodes)
			{
				DrawEntityNode(e);
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			for (Entity e : node.m_ChildNodes)
			{
				DeleteChildNode(e);
			}

			if (!entity.HasComponent<RootNodeTag>())
			{
				NodeComponent& parentComponent = node.m_ParentEntity.GetComponent<NodeComponent>();

				for (int i = 0; i < parentComponent.m_ChildNodes.size(); i++)
				{
					if (parentComponent.m_ChildNodes[i] == entity)
						parentComponent.m_ChildNodes.erase(parentComponent.m_ChildNodes.begin() + i);
				}
			}

			m_Scene->DestroyEntity(entity);

			if (m_Selected == entity)
				m_Selected = {};
		}
	}

	void SceneHierarchyPanel::DrawChildNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().tag;

		auto& node = entity.GetComponent<NodeComponent>();

		bool isLeaf = node.m_ChildNodes.size() == 0;

		ImGuiTreeNodeFlags flags = (m_Selected == entity ? ImGuiTreeNodeFlags_Selected : 0) |
									ImGuiTreeNodeFlags_OpenOnArrow |
									ImGuiTreeNodeFlags_OpenOnDoubleClick |
									(isLeaf ? ImGuiTreeNodeFlags_Leaf : 0) |
									ImGuiTreeNodeFlags_SpanAvailWidth;

		ImVec2 cursorPos = ImGui::GetCursorPos();

		bool opened = ImGui::TreeNodeEx((void*)(uint32_t)entity, flags, tag.c_str());

		int position = -1;
		DragProcedure(entity, position);

		ImVec2 cursorPosNow = ImGui::GetCursorPos();

		if(position == 1)
			ImGui::SetCursorPos(cursorPos);

		if (position != -1)
		{
			ImGui::PushStyleColor(ImGuiCol_Separator, { 1, 1, 0, 1 });
			ImGui::PushItemWidth(ImGui::GetItemRectSize().x);

			ImGui::Separator();

			ImGui::PopItemWidth();
			ImGui::PopStyleColor();
			ImGui::SetCursorPos(cursorPosNow);
		}

		ImGui::SetCursorPos({ ImGui::GetCursorPosX(), ImGui::GetCursorPosY() + 1});

		if (ImGui::IsItemClicked())
		{
			m_Selected = entity;
		}

		GImGui->CurrentWindow->DC.LastItemId = (uint32_t)entity;

		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (m_Selected == entity && ImGui::IsWindowHovered() && Input::IsKeyReleased(Key::Delete))
		{
			entityDeleted = true;
		}

		if (opened)
		{
			for (Entity& e : node.m_ChildNodes)
			{
				DrawChildNode(e);
			}
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			for (Entity& e : node.m_ChildNodes)
			{
				DeleteChildNode(e);
			}

			//TODO: Create better entity destruction
			if (!entity.HasComponent<RootNodeTag>())
			{
				NodeComponent& parentComponent = node.m_ParentEntity.GetComponent<NodeComponent>();

				for (int i = 0; i < parentComponent.m_ChildNodes.size(); i++)
				{
					if (parentComponent.m_ChildNodes[i] == entity)
						parentComponent.m_ChildNodes.erase(parentComponent.m_ChildNodes.begin() + i);
				}
			}

			m_Scene->DestroyEntity(entity);

			if (m_Selected == entity)
				m_Selected = {};
		}
	}

	void SceneHierarchyPanel::DeleteChildNode(Entity entity)
	{
		auto& node = entity.GetComponent<NodeComponent>();

		for (Entity& e : node.m_ChildNodes)
		{
			DeleteChildNode(e);
		}

		m_Scene->DestroyEntity(entity);
	}

	static void DrawFloat3Control(const std::string& label, DirectX::XMFLOAT3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2, 0, false);
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

		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Y
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		//Z
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | 
												 ImGuiTreeNodeFlags_AllowItemOverlap |
												 ImGuiTreeNodeFlags_Framed |
												 ImGuiTreeNodeFlags_SpanAvailWidth |
												 ImGuiTreeNodeFlags_FramePadding;

		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;

			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity.RemoveComponent<T>();
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().tag;

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, tag.c_str());

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			std::string entityIDText = "Entity ID: " + std::to_string(m_Scene->GetUUIDFromEntity(entity));

			ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(entityIDText.c_str()).x);
			ImGui::Text(entityIDText.c_str());
		}

		DrawComponent<TransformComponent>("Transform", entity, [](auto& transform)
		{
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
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& cameraComponent)
		{
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
		});
	}
}
