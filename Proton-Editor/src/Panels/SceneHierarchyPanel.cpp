#define NOMINMAX

#include "SceneHierarchyPanel.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"
#include "Proton\Core\Input.h"
#include "Proton\Core\KeyCodes.h"
#include "Proton\Asset Loader\AssetManager.h"
#include "AssetViewerPanel.h"
#include "Proton/Scripting/ScriptEngine.h"

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

	void SceneHierarchyPanel::SetContext(const Ref<Scene> scene)
	{
		Get().m_Context = scene;
		Get().m_Selected = {};
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName)
	{
		if (!m_Selected.HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				m_Selected.AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}

	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			auto& childView = m_Context->m_Registry.view<RootNodeTag>();

			for (auto e : childView)
			{
				DrawEntityNode(m_Context->m_Registry.get<IDComponent>(e).ID);
			}

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_Selected = {};

			//Right-Click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1, false))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("New Entity");

				ImGui::EndPopup();
			}
		}

		ImGui::End();

		ImGui::Begin("Inspector");

		if (m_Selected)
		{
			DrawComponents(m_Selected);

			ImGui::Separator();
			ImGui::Text("");
			ImGui::SameLine(ImGui::GetContentRegionAvail().x / 2.0f - 55);

			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("AddComponent");

			if (ImGui::BeginPopup("AddComponent"))
			{
				DisplayAddComponentEntry<CameraComponent>("Camera");
				DisplayAddComponentEntry<LightComponent>("Light");
				DisplayAddComponentEntry<ScriptComponent>("Script Component");

				ImGui::EndPopup();
			}
		}

		ImGui::End();
	}

	void SceneHierarchyPanel::DragProcedure(Entity entity, int& position)
	{
		position = -1;

		Ref<Scene> scene = Get().m_Context;
		UUID entityID = entity.GetUUID();

		if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::GetDragDropPayload() && ImGui::GetDragDropPayload()->IsDataType("SceneHierarchyObject"))
		{
			Entity payload = *(const Entity*)ImGui::GetDragDropPayload()->Data;
			if (payload == entity && !ImGui::IsDragDropPayloadBeingAccepted())
			{
				entity.SetParent();
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
						NodeComponent entityParentNode = scene->GetEntityByUUID(targetNode.ParentEntity).GetComponent<NodeComponent>();
						int pos = 0;
						for (int i = 0; i < entityParentNode.Children.size(); i++)
						{
							if (entityParentNode.Children[i] == entityID)
							{
								pos = i;
								break;
							}
						}

						draggedEntity.SetParent(targetNode.ParentEntity, pos);
					}
				}
				else if (percent > 0.75f)
				{
					position = 0;
					if (entity.HasComponent<RootNodeTag>())
					{
						draggedEntity.SetParent(entityID, 0);
					}
					else
					{
						NodeComponent entityParentNode = scene->GetEntityByUUID(targetNode.ParentEntity).GetComponent<NodeComponent>();
						int pos = 0;
						for (int i = 0; i < entityParentNode.Children.size(); i++)
						{
							if (entityParentNode.Children[i] == entityID)
							{
								pos = i;
								break;
							}
						}

						draggedEntity.SetParent(targetNode.ParentEntity, pos + 1);
					}
				}
				else
				{
					draggedEntity.SetParent(entityID);
				}
			}
			ImGui::EndDragDropTarget();
		}
	}

	void SceneHierarchyPanel::DrawEntityNode(UUID entityID)
	{
		Entity entity = m_Context->GetEntityByUUID(entityID);

		auto& tag = entity.GetComponent<TagComponent>().Tag;
		auto& node = entity.GetComponent<NodeComponent>();
		bool isLeaf = node.Children.size() == 0;

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
				std::string tag = entity.GetComponent<TagComponent>().Tag;

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
			for (UUID e : node.Children)
			{
				DrawEntityNode(e);
			}

			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			for (UUID e : node.Children)
			{
				DeleteChildNode(e);
			}

			if (!entity.HasComponent<RootNodeTag>())
			{
				NodeComponent& parentComponent = m_Context->GetEntityByUUID(node.ParentEntity).GetComponent<NodeComponent>();

				for (int i = 0; i < parentComponent.Children.size(); i++)
				{
					if (parentComponent.Children[i] == entityID)
						parentComponent.Children.erase(parentComponent.Children.begin() + i);
				}
			}

			m_Context->DestroyEntity(entity);

			if (m_Selected == entity)
				m_Selected = {};
		}
	}

	void SceneHierarchyPanel::DrawChildNode(UUID entityID)
	{
		Entity entity = m_Context->GetEntityByUUID(entityID);

		auto& tag = entity.GetComponent<TagComponent>().Tag;

		auto& node = entity.GetComponent<NodeComponent>();

		bool isLeaf = node.Children.size() == 0;

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
			for (UUID e : node.Children)
			{
				DrawChildNode(e);
			}
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			for (UUID e : node.Children)
			{
				DeleteChildNode(e);
			}

			//TODO: Create better entity destruction
			if (!entity.HasComponent<RootNodeTag>())
			{
				NodeComponent& parentComponent = m_Context->GetEntityByUUID(node.ParentEntity).GetComponent<NodeComponent>();

				for (int i = 0; i < parentComponent.Children.size(); i++)
				{
					if (parentComponent.Children[i] == entityID)
						parentComponent.Children.erase(parentComponent.Children.begin() + i);
				}
			}

			m_Context->DestroyEntity(entity);

			if (m_Selected == entity)
				m_Selected = {};
		}
	}

	void SceneHierarchyPanel::DeleteChildNode(UUID entityID)
	{
		auto& node = m_Context->GetEntityByUUID(entityID).GetComponent<NodeComponent>();

		for (UUID e : node.Children)
		{
			DeleteChildNode(e);
		}

		m_Context->DestroyEntity(m_Context->GetEntityByUUID(entityID));
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

	template<ScriptFieldType type, typename T>
	static bool DrawScriptField(const std::string& name, T* data)
	{
		if constexpr (type == ScriptFieldType::Float || type == ScriptFieldType::Double)
		{
			float castData = (float)*data;
			bool changed = ImGui::DragFloat(name.c_str(), &castData);
			*data = static_cast<T>(castData);
			return changed;
		}
		else if constexpr (type == ScriptFieldType::Bool)
		{
			bool changed = ImGui::Checkbox(name.c_str(), data);
			return changed;
		}
		else if constexpr (type == ScriptFieldType::Char)
		{
			//TODO: Memory leak
			static char castData[2];
			castData[0] = *data;
			bool changed = ImGui::InputText(name.c_str(), castData, sizeof(castData));
			*data = castData[0];
			return changed;
		}
		else if constexpr (type == ScriptFieldType::SByte || type == ScriptFieldType::Short ||
			type == ScriptFieldType::Int || type == ScriptFieldType::Long)
		{
			int min = std::numeric_limits<T>::min();
			int max = std::numeric_limits<T>::max();

			if constexpr (sizeof(T) > sizeof(int))
			{
				min = std::numeric_limits<int>::min();
				max = std::numeric_limits<int>::max();
			}

			int castData = (int)*data;
			bool changed = ImGui::DragInt(name.c_str(), &castData, 1.0f, min, max);
			*data = static_cast<T>(castData);
			return changed;
		}
		else if constexpr (type == ScriptFieldType::Byte || type == ScriptFieldType::UShort ||
						   type == ScriptFieldType::UInt || type == ScriptFieldType::ULong)
		{
			int max = std::numeric_limits<T>::max();
			if constexpr (std::numeric_limits<T>::max() > (T)std::numeric_limits<int>::max())
				constexpr int max = std::numeric_limits<int>::max();

			int castData = (int)*data;
			bool changed = ImGui::DragInt(name.c_str(), &castData, 1.0f, 0, max);
			*data = static_cast<T>(castData);
			return changed;
		}

		PT_CORE_ASSERT(false, "Field type not handled");
		return false;
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
			//float startCursorPosX = ImGui::GetCurrentWindow()->DC.CursorPos.x;

			auto& tag = entity.GetComponent<TagComponent>().Tag;

			char buffer[128];
			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, tag.c_str());

			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag = std::string(buffer);
			}

			std::string entityIDText = "Entity ID: " + fmt::format("{:x}", entity.GetUUID());
			ImGui::SameLine();
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

		DrawComponent<LightComponent>("Light", entity, [](auto& light)
		{
			ImGui::ColorPicker3("Ambient", &light.Ambient.x);
			ImGui::ColorPicker3("Diffuse", &light.DiffuseColour.x);
			ImGui::DragFloat("Diffuse Intensity", &light.DiffuseIntensity, 1.0f, 0.0f, 100.0f);
			ImGui::DragFloat("Attenuation Const", &light.AttConst, 0.5f, 0.0f, 100.0f);
			ImGui::DragFloat("Attenuation Linear", &light.AttLin, 0.5f, 0.0f, 100.0f);
			ImGui::DragFloat("Attenuation Quad", &light.AttQuad, 0.5f, 0.0f, 100.0f);
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& cameraComponent)
		{
			auto& camera = cameraComponent.Camera;

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
			else if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
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

		DrawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto& component) mutable
		{
			if (ImGui::Combo("Class", &component.ClassIndex, ScriptEngine::GetEntityClassNames(), ScriptEngine::GetEntityClasses().size(), -1))
			{
				component.ClassName = ScriptEngine::GetEntityClassNameByIndex(component.ClassIndex);
			}

			if (component.ClassName.empty())
				return;

			//Fields

			//If scene running
			bool isRunning = scene->IsRunning();
			if(isRunning)
			{
				Ref<ScriptInstance> scriptInstance = ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
				if (scriptInstance)
				{
					const auto& fields = scriptInstance->GetScriptClass()->GetFields();
					for (const auto [name, field] : fields)
					{
						int d = 0;
						switch (field.Type)
						{
#define X(scriptType, type) case ScriptFieldType::scriptType: \
							{\
								type data = scriptInstance->GetFieldValue<type>(name);	\
								if (DrawScriptField<ScriptFieldType::scriptType>(name, &data))\
								{\
									scriptInstance->SetFieldValue(name, data);\
								}\
							break;\
							}
							SCRIPT_FIELD_TYPES
#undef X
						}
					}
				}
			}
			else
			{
				Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(component.ClassName);
				const auto& fields = entityClass->GetFields();

				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				for (const auto [name, field] : fields)
				{
					//Field has been set in editor
					if (entityFields.find(name) != entityFields.end())
					{
						ScriptFieldInstance& scriptField = entityFields.at(name);
						
						switch (field.Type)
						{
#define X(scriptType, type) case ScriptFieldType::scriptType: \
							{\
								type data = scriptField.GetValue<type>(name);	\
								if (DrawScriptField<ScriptFieldType::scriptType>(name, &data))\
								{\
									scriptField.SetValue(data);\
								}\
							break;\
							}
							SCRIPT_FIELD_TYPES
#undef X
						}
					}
					else
					{
						switch (field.Type)
						{
#define X(scriptType, type) case ScriptFieldType::scriptType: \
							{\
								type data = ScriptEngine::GetScriptInstanceFromClass(component.ClassName).GetFieldValue<type>(name);	\
								if (DrawScriptField<ScriptFieldType::scriptType>(name, &data))\
								{\
									ScriptFieldInstance& fieldInstance = entityFields[name]; \
									fieldInstance.Field = field; \
									fieldInstance.SetValue<type>(data); \
								}\
							break;\
							}
							SCRIPT_FIELD_TYPES
#undef X
						}
					}
				}
			}
		});
	}
}
