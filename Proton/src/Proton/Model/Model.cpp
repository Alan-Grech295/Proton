#include "ptpch.h"
#include "Model.h"
#include "Proton/Scene/Components.h"
#include "Proton/Asset System/AssetManager.h"

//#include "Proton/Asset System/Loaders/ModelSerializer.h"

namespace Proton
{
	Entity Model::CreateEntity(Ref<Model> model, Scene& activeScene)
	{
		namespace dx = DirectX;

		Node& root = model->m_Nodes[0];

		// Hacky fix, TODO: Decide who will own the model

		return CreateNodeEntity(root, activeScene, model);
	}

	/*Ref<Model> Model::DeserializeEditor(const std::filesystem::path& path, UUID uuid)
	{
		return ModelSerializer::DeserializeModel(path, uuid);
	}*/

	Ref<Mesh> Model::GetMeshByName(const std::string& name)
	{
		for (Ref<Mesh> mesh : m_Meshes)
		{
			if (mesh->m_Name == name)
				return mesh;
		}

		PT_CORE_ASSERT(false, "Could not find mesh");
		return nullptr;
	}

	Ref<Material> Model::GetMaterialByName(const std::string& name)
	{
		for (Ref<Material> mat : m_DefaultMaterials)
		{
			if (mat->m_Name == name)
				return mat;
		}

		PT_CORE_ASSERT(false, "Could not find material");
		return nullptr;
	}

	Entity Proton::Model::CreateNodeEntity(Node& node, Scene& activeScene, Ref<Model> model)
	{
		//Node Creations
		namespace dx = DirectX;

		Entity childEntity = activeScene.CreateEntity(node.m_Name);
		UUID childID = childEntity.GetUUID();
		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.NodeName = node.m_Name;
		nodeComponent.Origin = node.m_Transformation;

		nodeComponent.Children.reserve(node.m_Children.size());

		if (node.m_Meshes.size() == 1)
		{
			AddMeshComponent(childEntity, model, node.m_Meshes[0]);
		}

		else if (node.m_Meshes.size() > 1)
		{
			for (Ref<Mesh> mesh : node.m_Meshes)
			{
				CreateMeshEntity(childEntity, activeScene, model, mesh);
			}
		}		

		for (Node* child : node.m_Children)
		{
			CreateNodeEntity(*child, activeScene, model).SetParent(childID);
		}

		return childEntity;
	}

	Entity Model::CreateMeshEntity(Entity parentEntity, Scene& activeScene, Ref<Model> modelRef, Ref<Mesh> mesh)
	{
		namespace dx = DirectX;

		Entity childEntity = activeScene.CreateEntity(mesh->m_Name);
		childEntity.SetParent(parentEntity);
		NodeComponent& nodeComponent = childEntity.GetComponent<NodeComponent>();
		nodeComponent.NodeName = mesh->m_Name;
		nodeComponent.Origin = dx::XMMatrixIdentity();

		AddMeshComponent(childEntity, modelRef, mesh);

		return childEntity;
	}

	void Model::AddMeshComponent(Entity entity, Ref<Model> modelRef, Ref<Mesh> mesh)
	{
		MeshRendererComponent& meshComponent = entity.AddComponent<MeshRendererComponent>();
		meshComponent.PMesh = mesh;

		meshComponent.Materials.push_back(mesh->m_DefaultMaterial);

		/*Ref<Material> pickMaterial = GetPickMaterial()->Clone();
		(*pickMaterial->GetBindable<PixelConstantBuffer>())["EntityID"] = (uint32_t)entity;

		meshComponent.Materials.push_back(pickMaterial);*/
		// TODO: Only run in editor mode
		//(*mesh->m_Materials[1]->GetBindable<PixelConstantBuffer>())["EntityID"] = (uint32_t)entity;
	}
}

