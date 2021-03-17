#pragma once
#include <DirectXMath.h>
#include "Proton\Renderer\Camera.h"
#include "Proton\Model\Model.h"
#include "Proton\Model\Model.h"
#include "ScriptableEntity.h"
#include "Proton\Scene\SceneCamera.h"
#include "Scene.h"
#include <vector>

namespace Proton
{
	class Entity;
	class Mesh;

	struct TransformComponent
	{
		DirectX::XMFLOAT3 position = { 0, 0, 0 };
		DirectX::XMFLOAT3 rotation = { 0, 0, 0 };
		DirectX::XMFLOAT3 scale = { 1, 1, 1 };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation)
			: 
			position(position),
			rotation(rotation) {}

		DirectX::XMMATRIX GetTransformMatrix()
		{
			return DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
				   DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
				   DirectX::XMMatrixTranslation(position.x, position.y, position.z);
		}
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
		SceneCamera camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
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
		Entity parentEntity;
		int numChildren = 0;

		//TEMP
		DirectX::XMMATRIX initialTransform;

		ChildNodeComponent() = default;
		ChildNodeComponent(const ChildNodeComponent&) = default;
		ChildNodeComponent(DirectX::FXMMATRIX& transform, Entity parent, std::vector<Entity> childNodes, int numChildren)
			: 
			initialTransform(transform),
			parentEntity(parent),
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

	struct LightComponent
	{
		DirectX::XMFLOAT3 ambient;
		DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;

		PixelConstantBuffer* cbuf;

		LightComponent(const LightComponent&) = default;
		LightComponent()
		{
			ambient = { 0.05f, 0.05f, 0.05f };
			diffuseColor = { 1.0f, 1.0f, 1.0f };
			diffuseIntensity = 2.0f;
			attConst = 1.0f;
			attLin = 0.045f;
			attQuad = 0.0075f;

			cbuf = PixelConstantBuffer::CreateUniquePtr(0, sizeof(Scene::PointLightData), new Scene::PointLightData());
		}
	};

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void(*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};
}