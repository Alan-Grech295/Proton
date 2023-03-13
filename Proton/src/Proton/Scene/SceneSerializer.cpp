#include "ptpch.h"
#include "SceneSerializer.h"
#include <cassert>
#include "Entity.h"
#include "Proton/Model/Model.h"
#include "Proton/Asset Loader/AssetCollection.h"
#include "Proton/Scripting/ScriptEngine.h"
#include "Proton/Core/UUID.h"

#include <yaml-cpp\yaml.h>
#include <fstream>

namespace YAML
{
	template<>
	struct convert<DirectX::XMFLOAT2>
	{
		static Node encode(const DirectX::XMFLOAT2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<DirectX::XMFLOAT3>
	{
		static Node encode(const DirectX::XMFLOAT3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
	
	template<>
	struct convert<DirectX::XMFLOAT4>
	{
		static Node encode(const DirectX::XMFLOAT4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, DirectX::XMFLOAT4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<DirectX::XMMATRIX>
	{
		static Node encode(const DirectX::XMMATRIX& rhs)
		{
			DirectX::XMFLOAT4X4 mat;
			DirectX::XMStoreFloat4x4(&mat, rhs);

			Node node;
			node.push_back(mat._11);	node.push_back(mat._12);	node.push_back(mat._13);	node.push_back(mat._14);
			node.push_back(mat._21);	node.push_back(mat._22);	node.push_back(mat._23);	node.push_back(mat._24);
			node.push_back(mat._31);	node.push_back(mat._32);	node.push_back(mat._33);	node.push_back(mat._34);
			node.push_back(mat._41);	node.push_back(mat._42);	node.push_back(mat._43);	node.push_back(mat._44);
			return node;
		}

		static bool decode(const Node& node, DirectX::XMMATRIX& rhs)
		{
			if (!node.IsSequence() || node.size() != 16)
				return false;

			DirectX::XMFLOAT4X4 mat;
			mat._11 = node[0].as<float>();	mat._12 = node[1].as<float>();	mat._13 = node[2].as<float>();	mat._14 = node[3].as<float>();
			mat._21 = node[4].as<float>();	mat._22 = node[5].as<float>();	mat._23 = node[6].as<float>();	mat._24 = node[7].as<float>();
			mat._31 = node[8].as<float>();	mat._32 = node[9].as<float>();	mat._33 = node[10].as<float>();	mat._34 = node[11].as<float>();
			mat._41 = node[12].as<float>();	mat._42 = node[13].as<float>();	mat._43 = node[14].as<float>();	mat._44 = node[15].as<float>();
			
			rhs = DirectX::XMLoadFloat4x4(&mat);
			return true;
		}
	};

	template<>
	struct convert<Proton::UUID>
	{
		static Node encode(const Proton::UUID& rhs)
		{

			Node node;
			node.push_back((uint64_t)rhs);
			return node;
		}

		static bool decode(const Node& node, Proton::UUID& rhs)
		{
			rhs = node.as<uint64_t>();
			return true;
		}
	};
}

namespace Proton
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMFLOAT4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const DirectX::XMMATRIX& m)
	{
		DirectX::XMFLOAT4X4 mat;
		DirectX::XMStoreFloat4x4(&mat, m);

		out << YAML::Flow;
		out << YAML::BeginSeq << mat._11 << mat._12 << mat._13 << mat._14 <<
								 mat._21 << mat._22 << mat._23 << mat._24 <<
								 mat._31 << mat._32 << mat._33 << mat._34 <<
								 mat._41 << mat._42 << mat._43 << mat._44 << YAML::EndSeq;
		return out;
	}

	SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
		:
		m_Scene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		PT_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap;	//Entity
		out << YAML::Key << "Entity" << YAML::Value << (uint64_t)entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;	//TagComponent

			auto& tag = entity.GetComponent<TagComponent>().Tag;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap;	//TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;	//TransformComponent

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << tc.position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.scale;

			out << YAML::EndMap;	//TransformComponent
		}

		bool writeMesh = true;

		if (entity.HasComponent<NodeComponent>())
		{
			out << YAML::Key << "NodeComponent";
			out << YAML::BeginMap;	//NodeComponent

			auto& nc = entity.GetComponent<NodeComponent>();
			//writeMesh = entity.HasComponent<RootNodeTag>() ? nc.m_PrefabName == "" : nc.m_RootEntity.GetComponent<NodeComponent>().m_PrefabName == "";
			out << YAML::Key << "Node Name" << YAML::Value << nc.NodeName;
			//out << YAML::Key << "Is Prefab" << YAML::Value << !writeMesh;
			//out << YAML::Key << "Prefab Path" << YAML::Value << nc.m_PrefabName;
			out << YAML::Key << "Parent Entity" << YAML::Value << (uint64_t)nc.ParentEntity;
			out << YAML::Key << "Root Entity" << YAML::Value << (uint64_t)nc.RootEntity;
			out << YAML::Key << "Initial Transform" << YAML::Value << nc.Origin;

			out << YAML::EndMap;	//NodeComponent
		}

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;	//LightComponent

			auto& lc = entity.GetComponent<LightComponent>();
			out << YAML::Key << "Ambient" << YAML::Value << lc.Ambient;
			out << YAML::Key << "Diffuse Color" << YAML::Value << lc.DiffuseColour;
			out << YAML::Key << "Diffuse Intensity" << YAML::Value << lc.DiffuseIntensity;
			out << YAML::Key << "Attenuation Constant" << YAML::Value << lc.AttConst;
			out << YAML::Key << "Attenuation Linear" << YAML::Value << lc.AttLin;
			out << YAML::Key << "Attenuation Quadratic" << YAML::Value << lc.AttQuad;

			out << YAML::EndMap;	//LightComponent
		}

		if (entity.HasComponent<MeshComponent>() && writeMesh)
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;	//MeshComponent

			auto& mc = entity.GetComponent<MeshComponent>();

			for (int i = 0; i < mc.MeshPtrs.size(); i++)
			{
				Mesh& mesh = *mc.MeshPtrs[i];
				out << YAML::Key << ("Mesh" + std::to_string(i));

				out << YAML::BeginMap;	//Mesh
				out << YAML::Key << "Model Path" << YAML::Value << mesh.m_ModelPath;
				out << YAML::Key << "Mesh Name" << YAML::Value << mesh.m_Name;
				out << YAML::EndMap;	//Mesh
			}

			out << YAML::EndMap;	//MeshComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;	//CameraComponent

			auto& cc = entity.GetComponent<CameraComponent>();
			auto& camera = cc.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;	//Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;	//Camera

			out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.FixedAspectRatio;

			out << YAML::EndMap;	//CameraComponent
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap;	//NativeScriptComponent

			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			out << YAML::Key << "Type" << YAML::Value << typeid(*nsc.Instance).name();
			
			out << YAML::EndMap;	//NativeScriptComponent
		}

		if (entity.HasComponent<ScriptComponent>())
		{
			out << YAML::Key << "ScriptComponent";
			out << YAML::BeginMap;	// ScriptComponent

			auto& scriptComponent = entity.GetComponent<ScriptComponent>();
			out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

			//Fields
			Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
			const auto& fields = entityClass->GetFields();
			if (fields.size() > 0)
			{
				out << YAML::Key << "ScriptFields" << YAML::Value;
				auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
				out << YAML::BeginSeq;		// Script Fields

				for (const auto [name, field] : fields)
				{
					if(entityFields.find(name) == entityFields.end())
						continue;

					out << YAML::BeginMap;	// ScriptField
					out << YAML::Key << "Name" << YAML::Value << name;
					out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);
					out << YAML::Key << "Data" << YAML::Value;

					ScriptFieldInstance& scriptField = entityFields.at(name);

					switch (field.Type)
					{
#define X(typeName, type) case ScriptFieldType::typeName:				\
							out << scriptField.GetValue<type>(name);	\
							break;
						SCRIPT_FIELD_TYPES
#undef X
					}

					out << YAML::EndMap;

				}
				out << YAML::EndSeq;		// Script Fields
			}

			out << YAML::EndMap;	// ScriptComponent
		}

		out << YAML::EndMap;	//Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath, const EditorCamera& editorCam)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";

		out << YAML::Key << "Editor Camera";

		out << YAML::BeginMap;
		XMFLOAT3 focalPoint;
		DirectX::XMStoreFloat3(&focalPoint, editorCam.m_FocalPoint);

		out << YAML::Key << "Focal Point" << YAML::Value << focalPoint;
		out << YAML::Key << "Distance" << YAML::Value << editorCam.m_Distance;
		out << YAML::Key << "Yaw" << YAML::Value << editorCam.m_Yaw;
		out << YAML::Key << "Pitch" << YAML::Value << editorCam.m_Pitch;
		out << YAML::Key << "FOV" << YAML::Value << editorCam.m_FOV;
		out << YAML::Key << "Near Clip" << YAML::Value << editorCam.m_NearClip;
		out << YAML::Key << "Far Clip" << YAML::Value << editorCam.m_FarClip;
		out << YAML::EndMap;

		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

		auto& view = m_Scene->m_Registry.view<NodeComponent>();

		auto it = view.rbegin();

		while (it != view.rend())
		{
			entt::entity entityID = *it;

			Entity entity(entityID, m_Scene.get());
			if (!entity)
				return;

			SerializeEntity(out, entity);

			it++;
		}
		
		out << YAML::EndSeq;
		out << YAML::EndMap;
		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		assert("Not Implemented!");
	}

	//TODO: Check entity order when loading
	bool SceneSerializer::Deserialize(const std::filesystem::path& filepath, EditorCamera& editorCamera)
	{
		m_Scene->ClearEntities();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			PT_CORE_ERROR("Failed to load .proton file '{0'}\n	{1}", filepath.string(), e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		PT_CORE_TRACE("Deserializing scene {0}", sceneName);

		auto editorCam = data["Editor Camera"];
		if (editorCam)
		{
			editorCamera.m_FocalPoint = XMLoadFloat3(&editorCam["Focal Point"].as<XMFLOAT3>());
			editorCamera.m_Distance = editorCam["Distance"].as<float>();
			editorCamera.m_Yaw = editorCam["Yaw"].as<float>();
			editorCamera.m_Pitch = editorCam["Pitch"].as<float>();
			editorCamera.m_FOV = editorCam["FOV"].as<float>();
			editorCamera.m_NearClip = editorCam["Near Clip"].as<float>();
			editorCamera.m_FarClip = editorCam["Far Clip"].as<float>();
			editorCamera.UpdateView();
		}

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				PT_CORE_TRACE("Deserialized entity with ID: {0}, name: {1}", uuid, name);

				Entity deserializedEntity = m_Scene->CreateEntityWithUUID(uuid, name);

				//Transform Component
				TransformComponent& transformComponent = deserializedEntity.GetComponent<TransformComponent>();
				auto transformNode = entity["TransformComponent"];

				if (transformNode)
				{
					transformComponent.position = transformNode["Position"].as<DirectX::XMFLOAT3>();
					transformComponent.rotation = transformNode["Rotation"].as<DirectX::XMFLOAT3>();
					transformComponent.scale = transformNode["Scale"].as<DirectX::XMFLOAT3>();
				}

				//NodeComponent
				auto node = entity["NodeComponent"];
				if (node)
				{
					NodeComponent& nodeComponent = deserializedEntity.GetComponent<NodeComponent>();

					nodeComponent.Origin = node["Initial Transform"].as<DirectX::XMMATRIX>();

					UUID parentUUID = node["Parent Entity"].as<uint64_t>();

					if (parentUUID != UUID::Null)
					{
						deserializedEntity.SetParent(parentUUID);
						nodeComponent.RootEntity = node["Root Entity"].as<uint64_t>();
					}

					//childNodeComponent.m_PrefabName = node["Prefab Path"].as<std::string>();
					nodeComponent.NodeName = node["Node Name"].as<std::string>();
				}

				/*if (node["Is Prefab"].as<bool>())
				{
					MeshComponent& meshComponent = deserializedEntity.AddComponent<MeshComponent>();
					PrefabNode* node = ModelCollection::GetPrefabNode(childNodeComponent.m_RootEntity.GetComponent<NodeComponent>().m_PrefabName, childNodeComponent.m_NodeName);

					for (int i = 0; i < node->numMeshes; i++)
					{
						meshComponent.m_MeshPtrs.push_back(node->meshes[i]);
						meshComponent.m_NumMeshes++;
					}
				}*/

				//MeshComponent
				auto meshNode = entity["MeshComponent"];

				if (meshNode)
				{
					MeshComponent& meshComponent = deserializedEntity.AddComponent<MeshComponent>();
					NodeComponent& nodeComponent = deserializedEntity.GetComponent<NodeComponent>();

					uint32_t i = 0;
					YAML::Node meshDataNode;
					while (meshDataNode = meshNode["Mesh" + std::to_string(i)])
					{
						std::string modelPath = meshDataNode["Model Path"].as<std::string>();
						std::string meshName = meshDataNode["Mesh Name"].as<std::string>();

						Ref<Model> model = ModelCollection::GetOrCreate(nodeComponent.RootEntity, modelPath);
						
						meshComponent.MeshPtrs.push_back(model->FindMeshWithName(meshName));

						i++;
					}
				}

				//RootNodeComponent
				/*auto rootNode = entity["RootNodeComponent"];
				
				if (rootNode)
				{
					RootNodeComponent& rootNodeComponent = deserializedEntity.AddComponent<RootNodeComponent>();

					rootNodeComponent.initialTransform = rootNode["Initial Transform"].as<DirectX::XMMATRIX>();
					rootNodeComponent.prefabPath = rootNode["Prefab Path"].as<std::string>();
					rootNodeComponent.nodeName = rootNode["Node Name"].as<std::string>();

					if (rootNodeComponent.prefabPath != "")
					{
						MeshComponent& meshComponent = deserializedEntity.AddComponent<MeshComponent>();
						PrefabNode* node = ModelCollection::GetPrefabNode(rootNodeComponent.prefabPath, rootNodeComponent.nodeName);

						for (int i = 0; i < node->numMeshes; i++)
						{
							meshComponent.m_MeshPtrs.push_back(node->meshes[i]);
							meshComponent.m_NumMeshes++;
						}
					}
				}*/
				
				//LightComponent
				auto lightNode = entity["LightComponent"];

				if (lightNode)
				{
					LightComponent& lightComponent = deserializedEntity.AddComponent<LightComponent>();

					lightComponent.Ambient = lightNode["Ambient"].as<DirectX::XMFLOAT3>();
					lightComponent.DiffuseColour = lightNode["Diffuse Color"].as<DirectX::XMFLOAT3>();
					lightComponent.DiffuseIntensity = lightNode["Diffuse Intensity"].as<float>();
					lightComponent.AttConst = lightNode["Attenuation Constant"].as<float>();
					lightComponent.AttLin = lightNode["Attenuation Linear"].as<float>();
					lightComponent.AttQuad = lightNode["Attenuation Quadratic"].as<float>();
				}
				
				//CameraComponent
				auto cameraNode = entity["CameraComponent"];

				if (cameraNode)
				{
					CameraComponent& cameraComponent = deserializedEntity.AddComponent<CameraComponent>();

					auto cameraSettingsNode = cameraNode["Camera"];
					cameraComponent.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraSettingsNode["ProjectionType"].as<int>());
					cameraComponent.Camera.SetPerspectiveVerticalFOV(cameraSettingsNode["PerspectiveFOV"].as<float>());
					cameraComponent.Camera.SetPerspectiveNearClip(cameraSettingsNode["PerspectiveNear"].as<float>());
					cameraComponent.Camera.SetPerspectiveFarClip(cameraSettingsNode["PerspectiveFar"].as<float>());

					cameraComponent.Camera.SetOrthographicSize(cameraSettingsNode["OrthographicSize"].as<float>());
					cameraComponent.Camera.SetOrthographicNearClip(cameraSettingsNode["OrthographicNear"].as<float>());
					cameraComponent.Camera.SetOrthographicFarClip(cameraSettingsNode["OrthographicFar"].as<float>());

					cameraComponent.Primary = cameraNode["Primary"].as<bool>();
					cameraComponent.FixedAspectRatio = cameraNode["FixedAspectRatio"].as<bool>();
				}

				//Script Component
				auto scriptComponent = entity["ScriptComponent"];

				if (scriptComponent)
				{
					ScriptComponent& sc = deserializedEntity.AddComponent<ScriptComponent>();

					sc.ClassName = scriptComponent["ClassName"].as<std::string>();
					sc.ClassIndex = ScriptEngine::GetEntityClassIndexFromName(sc.ClassName);

					auto scriptFields = scriptComponent["ScriptFields"];
					if (scriptFields)
					{
						Ref<ScriptClass> entityClass = ScriptEngine::GetEntityClass(sc.ClassName);
						PT_CORE_ASSERT(entityClass);
						const auto& fields = entityClass->GetFields();
						auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

						for (auto scriptField : scriptFields)
						{
							std::string name = scriptField["Name"].as<std::string>();
							std::string	typeString = scriptField["Type"].as<std::string>();
							ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);
							
							ScriptFieldInstance& fieldInstance = entityFields[name];
							//TODO: Turn assert into Editor log message
							PT_CORE_ASSERT(fields.find(name) != fields.end());
							if(fields.find(name) == fields.end())
								continue;

							fieldInstance.Field = fields.at(name);

							switch (type)
							{
#define X(typeName, t) case ScriptFieldType::typeName:					\
							{											\
								t data = scriptField["Data"].as<t>();	\
								fieldInstance.SetValue(data);			\
							}											\
							break;
								SCRIPT_FIELD_TYPES
#undef X
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		assert("Not Implemented!");
		return false;
	}
}