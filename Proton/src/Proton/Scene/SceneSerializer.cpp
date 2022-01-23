#include "ptpch.h"
#include "SceneSerializer.h"
#include <cassert>
#include "Entity.h"
#include "Components.h"
#include "Model.h"
#include "ModelCollection.h"

#include <yaml-cpp\yaml.h>
#include <fstream>

namespace YAML
{
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
}

namespace Proton
{
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
		out << YAML::BeginMap;	//Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;	//TagComponent

			auto& tag = entity.GetComponent<TagComponent>().tag;
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
			writeMesh = entity.HasComponent<RootNodeTag>() ? nc.m_PrefabName == "" : nc.m_RootEntity.GetComponent<NodeComponent>().m_PrefabName == "";
			out << YAML::Key << "Node Name" << YAML::Value << nc.m_NodeName;
			out << YAML::Key << "Is Prefab" << YAML::Value << !writeMesh;
			out << YAML::Key << "Prefab Path" << YAML::Value << nc.m_PrefabName;
			out << YAML::Key << "Parent Entity" << YAML::Value << (nc.m_ParentEntity == Entity::Null ? LLONG_MAX : nc.m_ParentEntity.GetUUID());
			out << YAML::Key << "Root Entity" << YAML::Value << (nc.m_RootEntity == Entity::Null ? LLONG_MAX : nc.m_RootEntity.GetUUID());
			out << YAML::Key << "Initial Transform" << YAML::Value << nc.m_Origin;

			out << YAML::EndMap;	//NodeComponent
		}

		if (entity.HasComponent<LightComponent>())
		{
			out << YAML::Key << "LightComponent";
			out << YAML::BeginMap;	//LightComponent

			auto& lc = entity.GetComponent<LightComponent>();
			out << YAML::Key << "Ambient" << YAML::Value << lc.ambient;
			out << YAML::Key << "Diffuse Color" << YAML::Value << lc.diffuseColor;
			out << YAML::Key << "Diffuse Intensity" << YAML::Value << lc.diffuseIntensity;
			out << YAML::Key << "Attenuation Constant" << YAML::Value << lc.attConst;
			out << YAML::Key << "Attenuation Linear" << YAML::Value << lc.attLin;
			out << YAML::Key << "Attenuation Quadratic" << YAML::Value << lc.attQuad;

			out << YAML::EndMap;	//LightComponent
		}

		if (entity.HasComponent<MeshComponent>() && writeMesh)
		{
			out << YAML::Key << "MeshComponent";
			out << YAML::BeginMap;	//MeshComponent

			auto& mc = entity.GetComponent<MeshComponent>();

			for (int i = 0; i < mc.m_NumMeshes; i++)
			{
				Mesh& mesh = *mc.m_MeshPtrs[i];
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
			auto& camera = cc.camera;

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

		out << YAML::EndMap;	//Entity
	}

	static void SerializeChild(YAML::Emitter& out, Entity entity)
	{
		NodeComponent& cc = entity.GetComponent<NodeComponent>();

		SerializeEntity(out, entity);

		for (Entity child : cc.m_ChildNodes)
		{
			SerializeChild(out, child);
		}
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

		auto& view = m_Scene->m_Registry.view<RootNodeTag>();

		auto it = view.rbegin();

		while (it != view.rend())
		{
			entt::entity entityID = *it;

			Entity entity{ entityID, m_Scene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);

			NodeComponent& root = entity.GetComponent<NodeComponent>();

			for (Entity child : root.m_ChildNodes)
			{
				SerializeChild(out, child);
			}

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

	bool SceneSerializer::Deserialize(const std::string& filepath, EditorCamera& editorCamera)
	{
		m_Scene->ClearEntities();

		std::ifstream stream(filepath);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		YAML::Node data = YAML::Load(strStream.str());
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

				transformComponent.position = transformNode["Position"].as<DirectX::XMFLOAT3>();
				transformComponent.rotation = transformNode["Rotation"].as<DirectX::XMFLOAT3>();
				transformComponent.scale = transformNode["Scale"].as<DirectX::XMFLOAT3>();

				//NodeComponent
				auto node = entity["NodeComponent"];
				NodeComponent& childNodeComponent = deserializedEntity.GetComponent<NodeComponent>();

				childNodeComponent.m_Origin = node["Initial Transform"].as<DirectX::XMMATRIX>();

				if (node["Parent Entity"].as<uint64_t>() != LLONG_MAX)
				{
					Entity& parent = m_Scene->GetEntityFromUUID(node["Parent Entity"].as<uint64_t>());
					deserializedEntity.SetParent(&parent);

					Entity& root = m_Scene->GetEntityFromUUID(node["Root Entity"].as<uint64_t>());
					childNodeComponent.m_RootEntity = root;
				}

				childNodeComponent.m_PrefabName = node["Prefab Path"].as<std::string>();
				childNodeComponent.m_NodeName = node["Node Name"].as<std::string>();

				if (node["Is Prefab"].as<bool>())
				{
					MeshComponent& meshComponent = deserializedEntity.AddComponent<MeshComponent>();
					PrefabNode* node = ModelCollection::GetPrefabNode(childNodeComponent.m_RootEntity.GetComponent<NodeComponent>().m_PrefabName, childNodeComponent.m_NodeName);

					for (int i = 0; i < node->numMeshes; i++)
					{
						meshComponent.m_MeshPtrs.push_back(node->meshes[i]);
						meshComponent.m_NumMeshes++;
					}
				}

				//MeshComponent
				auto meshNode = entity["MeshComponent"];

				if (meshNode)
				{
					MeshComponent& meshComponent = deserializedEntity.AddComponent<MeshComponent>();

					uint32_t i = 0;
					YAML::Node meshDataNode;
					while (meshDataNode = meshNode["Mesh" + std::to_string(i)])
					{
						std::string modelPath = meshDataNode["Model Path"].as<std::string>();
						std::string meshName = meshDataNode["Mesh Name"].as<std::string>();

						meshComponent.m_MeshPtrs.push_back(ModelCollection::GetMesh(modelPath, meshName));
						meshComponent.m_NumMeshes++;

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

					lightComponent.ambient = lightNode["Ambient"].as<DirectX::XMFLOAT3>();
					lightComponent.diffuseColor = lightNode["Diffuse Color"].as<DirectX::XMFLOAT3>();
					lightComponent.diffuseIntensity = lightNode["Diffuse Intensity"].as<float>();
					lightComponent.attConst = lightNode["Attenuation Constant"].as<float>();
					lightComponent.attLin = lightNode["Attenuation Linear"].as<float>();
					lightComponent.attQuad = lightNode["Attenuation Quadratic"].as<float>();
				}
				
				//CameraComponent
				auto cameraNode = entity["CameraComponent"];

				if (cameraNode)
				{
					CameraComponent& cameraComponent = deserializedEntity.AddComponent<CameraComponent>();

					auto cameraSettingsNode = cameraNode["Camera"];
					cameraComponent.camera.SetProjectionType((SceneCamera::ProjectionType)cameraSettingsNode["ProjectionType"].as<int>());
					cameraComponent.camera.SetPerspectiveVerticalFOV(cameraSettingsNode["PerspectiveFOV"].as<float>());
					cameraComponent.camera.SetPerspectiveNearClip(cameraSettingsNode["PerspectiveNear"].as<float>());
					cameraComponent.camera.SetPerspectiveFarClip(cameraSettingsNode["PerspectiveFar"].as<float>());

					cameraComponent.camera.SetOrthographicSize(cameraSettingsNode["OrthographicSize"].as<float>());
					cameraComponent.camera.SetOrthographicNearClip(cameraSettingsNode["OrthographicNear"].as<float>());
					cameraComponent.camera.SetOrthographicFarClip(cameraSettingsNode["OrthographicFar"].as<float>());

					cameraComponent.Primary = cameraNode["Primary"].as<bool>();
					cameraComponent.FixedAspectRatio = cameraNode["FixedAspectRatio"].as<bool>();
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