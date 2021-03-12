#pragma once
#include <DirectXMath.h>
#include "Proton\Renderer\Camera.h"
#include "Proton\Model\Model.h"
#include "Proton\Model\Model.h"
//#include "Proton\Scene\Entity.h"
#include <vector>

namespace Proton
{
	class Entity;
	class Mesh;
	struct TransformComponent
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation)
			: 
			position(position),
			rotation(rotation) {}
	};

	struct TagComponent
	{
		std::string tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: tag(tag) {}
	};

	struct CameraComponent
	{
		Camera camera;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
		CameraComponent(DirectX::FXMMATRIX& projection)
			: camera(projection) {}
	};

	struct MeshComponent
	{
		std::vector<Mesh*> m_MeshPtrs;
		int m_NumMeshes = 0;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;

		MeshComponent(std::vector<Mesh*> meshPtrs, int numMeshes)
			:
			m_MeshPtrs(meshPtrs),
			m_NumMeshes(numMeshes){}
	};

	struct ChildNodeComponent
	{
		std::vector<Entity> childNodes;
		int numChildren = 0;

		//TEMP
		DirectX::XMMATRIX initialTransform;

		ChildNodeComponent() = default;
		ChildNodeComponent(const ChildNodeComponent&) = default;
		ChildNodeComponent(DirectX::FXMMATRIX& transform, std::vector<Entity> childNodes, int numChildren)
			: 
			initialTransform(transform),
			childNodes(childNodes),
			numChildren(numChildren) {}
	};

	struct ParentNodeComponent
	{
		std::vector<Entity> childNodes;
		std::vector<Mesh*> meshPtrs;
		int numChildren = 0;

		//TEMP
		DirectX::XMMATRIX initialTransform;

		ParentNodeComponent()
			:
			childNodes(std::vector<Entity>()),
			numChildren(0)
		{
		}

		ParentNodeComponent(const ParentNodeComponent&) = default;
		ParentNodeComponent(DirectX::FXMMATRIX& transform, std::vector<Entity> childNodes, int numChildren)
			:
			initialTransform(transform),
			childNodes(childNodes),
			numChildren(numChildren) {}
	};
}