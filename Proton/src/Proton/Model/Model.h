#pragma once
#include "Mesh.h"
#include "Proton/Asset System/Asset.h"
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

	class Model : public Asset
	{
	public:
		Model(UUID uuid) 
			: Asset(uuid) 
		{}

		Entity CreateEntity(Scene& activeScene);

		static Ref<Model> DeserializeEditor(const std::filesystem::path& path, UUID uuid);
	private:
		Entity CreateNodeEntity(Node& node, Scene& activeScene);
	public:
		// Nodes and meshes are model specific
		std::vector<Node> m_Nodes;
		std::vector<Mesh> m_Meshes;
		// Note: Materials are not being stored in the model as different materials
		// can be used in different models 
	};
}


