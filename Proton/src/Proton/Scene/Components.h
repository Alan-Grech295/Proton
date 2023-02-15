#pragma once

//#include "ScriptableEntity.h"
#include "SceneCamera.h"
#include "Scene.h"
#include "Proton/Core/UUID.h"

#include <vector>
#include <DirectXMath.h>
#include <string.h>

namespace Proton
{
	//Forward declaration
	class Entity;

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(UUID uuid)
			:
			ID(uuid)
		{}

		IDComponent(const IDComponent&) = default;
	};

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
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true;
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct MeshComponent
	{
		MeshComponent() = default;
		MeshComponent(const MeshComponent& other) = default;

		/*MeshComponent(std::vector<Mesh*> meshPtrs)
			:
			m_MeshPtrs(meshPtrs)
		{}*/

	public:
		std::vector<class Mesh*> MeshPtrs;
		Ref<class Model> ModelRef;
	};

	struct RootNodeTag
	{
		RootNodeTag() = default;
		RootNodeTag(const RootNodeTag& other) = default;

	private:
		bool placeholder;
	};

	struct NodeComponent
	{
		std::vector<Entity> Children;
		Entity ParentEntity;
		Entity RootEntity;
		std::string NodeName;
		//std::string m_PrefabName;

		DirectX::XMMATRIX Origin;

		NodeComponent()
			:
			ParentEntity(Entity::Null),
			RootEntity(Entity::Null),
			NodeName(""),
			//m_PrefabName(""),
			Origin(DirectX::XMMatrixIdentity())
		{};

		NodeComponent(const NodeComponent&) = default;
		//NodeComponent(const std::string& nodeName, const std::string& prefabName, Entity parentEntity, Entity rootEntity, DirectX::FXMMATRIX& origin)
		//	:
		//	m_NodeName(nodeName),
		//	//m_PrefabName(prefabName),
		//	m_ParentEntity(parentEntity),
		//	m_RootEntity(rootEntity),
		//	m_Origin(origin)
		//{}
	};

	/*struct ModelComponent
	{
		ModelComponent() = default;
		ModelComponent(const ModelComponent&) = default;

		ModelComponent(std::vector<Mesh> meshes, std::vector<class Material> materials)
			:
			m_Meshes(meshes),
			m_Materials(materials)
		{
		}
	public:
		std::vector<Mesh> m_Meshes;
		std::vector<class Material> m_Materials;

		DirectX::XMMATRIX m_Origin;
	};*/

	struct LightComponent
	{
		DirectX::XMFLOAT3 Ambient;
		DirectX::XMFLOAT3 DiffuseColour;
		float DiffuseIntensity;
		float AttConst;
		float AttLin;
		float AttQuad;

		LightComponent(const LightComponent&) = default;
		LightComponent()
		{
			Ambient = { 0.05f, 0.05f, 0.05f };
			DiffuseColour = { 1.0f, 1.0f, 1.0f };
			DiffuseIntensity = 2.0f;
			AttConst = 1.0f;
			AttLin = 0.045f;
			AttQuad = 0.0075f;

			//cbuf = PixelConstantBuffer::CreateUniquePtr(0, sizeof(Scene::PointLightData), new Scene::PointLightData());
		}
	};

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	//Forward declaration
	class ScriptableEntity;

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