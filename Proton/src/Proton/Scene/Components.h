#pragma once
#include <DirectXMath.h>
#include "Proton\Renderer\Camera.h"
#include "Proton\Model\Model.h"
#include "ScriptableEntity.h"
#include "Proton\Scene\SceneCamera.h"
#include "Scene.h"
#include <vector>

namespace Proton
{
	class Entity;
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
		uint32_t m_NumMeshes = 0;

		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;

		MeshComponent(std::vector<Mesh*> meshPtrs, uint32_t numMeshes)
			:
			m_MeshPtrs(meshPtrs),
			m_NumMeshes(numMeshes){}
	};

	struct NodeComponent
	{
		std::vector<Entity> m_ChildNodes;
		Entity m_ParentEntity;
		Entity m_RootEntity;
		std::string m_NodeName;
		std::string m_PrefabName;

		DirectX::XMMATRIX m_Origin;

		NodeComponent()
			:
			m_ParentEntity(Entity::Null),
			m_RootEntity(Entity::Null),
			m_NodeName(""),
			m_PrefabName("")
		{};
		NodeComponent(const NodeComponent&) = default;
		NodeComponent(const std::string& nodeName, const std::string& prefabName, Entity parentEntity, Entity rootEntity, DirectX::FXMMATRIX& origin)
			:
			m_NodeName(nodeName),
			m_PrefabName(prefabName),
			m_ParentEntity(parentEntity),
			m_RootEntity(rootEntity),
			m_Origin(origin)
		{}
	};

	struct RootNodeTag 
	{
		RootNodeTag() = default;
		RootNodeTag(const RootNodeTag&) = default;
	private:
		bool placeholder;
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