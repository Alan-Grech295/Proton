#pragma once
#include "Mesh.h"
#include "Proton/Scene/Entity.h"

namespace Proton
{
	class Node
	{
	public:

	public:
		std::string m_Name;

		// Since the node does not own the meshes or child nodes as they are stored in the model
		// they can be stored as pointers
		std::vector<Mesh*> m_Meshes;
		std::vector<Node*> m_Children;

		DirectX::XMMATRIX m_Transformation;
	};

	class Model
	{
	public:
		Model()
		{}

		static Entity CreateEntity(Ref<Model> model, Scene& activeScene);

		~Model()
		{
			m_Meshes.~vector();
			m_Nodes.~vector();
		}

		//static Ref<Model> DeserializeEditor(const std::filesystem::path& path, UUID uuid);
	private:
		static Entity CreateNodeEntity(Node& node, Scene& activeScene, Ref<Model> model);
		static Entity CreateMeshEntity(Entity parentEntity, Scene& activeScene, Ref<Model> modelRef, Mesh* mesh);
		static void AddMeshComponent(Entity entity, Ref<Model> modelRef, Mesh* mesh);
	public:
		// Nodes and meshes are model specific
		std::vector<Node> m_Nodes;
		std::vector<Mesh> m_Meshes;
		// Note: Materials are not being stored in the model as different materials
		// can be used in different models 
	};
}


