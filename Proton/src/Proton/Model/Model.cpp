#include "ptpch.h"
#include "Model.h"
#include "Proton/Scene/Components.h"
#include "Proton/Asset System/AssetManager.h"

#include "Proton/Asset System/Loaders/ModelSerializer.h"

namespace Proton
{
	Entity Model::CreateEntity(Scene& activeScene)
	{
		namespace dx = DirectX;

		Node& root = m_Nodes[0];

		return CreateNodeEntity(root, activeScene);
	}

	Ref<Model> Model::DeserializeEditor(const std::filesystem::path& path, UUID uuid)
	{
		return ModelSerializer::DeserializeModel(path, uuid);
	}

	Entity Proton::Model::CreateNodeEntity(Node& node, Scene& activeScene)
	{
		//Node Creations
		namespace dx = DirectX;

		Entity childEntity = activeScene.CreateEntity(node.m_Name);
		UUID childID = childEntity.GetUUID();
		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.NodeName = node.m_Name;
		nodeComponent.Origin = node.m_Transformation;

		nodeComponent.Children.reserve(node.m_Children.size());

		StaticMeshComponent& meshComponent = childEntity.AddComponent<StaticMeshComponent>();
		meshComponent.ModelRef = AssetManager::GetEditorAsset<Model>(m_UUID);

		meshComponent.MeshPtrs.reserve(node.m_Meshes.size());

		for (Mesh* mesh : node.m_Meshes)
		{
			meshComponent.MeshPtrs.push_back(mesh);
		}

		for (Node* child : node.m_Children)
		{
			CreateNodeEntity(*child, activeScene).SetParent(childID);
		}

		return childEntity;
	}
}

