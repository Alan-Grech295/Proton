#pragma once

//#include "ScriptableEntity.h"
#include "SceneCamera.h"
#include "Scene.h"
#include "Proton/Core/UUID.h"
#include "Proton/Model/Mesh.h"

#include <vector>
#include <DirectXMath.h>
#include <string.h>

namespace Proton
{
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
		DirectX::XMVECTOR rotQuaternion = DirectX::XMQuaternionIdentity();
		DirectX::XMFLOAT3 scale = { 1, 1, 1 };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& rotation)
			: 
			position(position),
			rotation(rotation) {}

		DirectX::XMMATRIX GetLocalTransformMatrix()
		{
			return DirectX::XMMatrixScaling(scale.x, scale.y, scale.z) *
				   DirectX::XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
				   //DirectX::XMMatrixRotationQuaternion(rotQuaternion) *
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

	struct MeshRendererComponent
	{
		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};

		MeshRendererComponent()
		{
			TransformBufferVert = VertexConstantBuffer::CreateUnique(0, sizeof(Transforms), new Transforms());
			DCB::RawLayout layout;
			layout.Add(DCB::Type::Matrix4x4, "modelViewProj");
			layout.Add(DCB::Type::Matrix4x4, "model");
			TransformBufferPix = PixelConstantBuffer::CreateUnique(2, DCB::CookedLayout(std::move(layout)));
		}

		// Copy constructor
		MeshRendererComponent(const MeshRendererComponent& other)
			: PMesh(other.PMesh), Materials(other.Materials)
		{
			TransformBufferVert = VertexConstantBuffer::CreateUnique(other.TransformBufferVert.get());
			TransformBufferPix = PixelConstantBuffer::CreateUnique(other.TransformBufferPix.get());
		}

		/*MeshRendererComponent(std::vector<Mesh*> meshPtrs)
			:
			m_MeshPtrs(meshPtrs)
		{}*/

	public:
		Ref<Mesh> PMesh = nullptr;

		std::vector<Ref<Material>> Materials;

		Ref<VertexConstantBuffer> TransformBufferVert;
		Ref<PixelConstantBuffer> TransformBufferPix;
	};

	struct RootNodeTag
	{
		RootNodeTag() = default;
		RootNodeTag(const RootNodeTag& other) = default;

	private:
		bool placeholder;
	};

	//Check if better to convert to Entity
	struct NodeComponent
	{
		std::vector<UUID> Children;
		UUID ParentEntity;
		UUID RootEntity;
		std::string NodeName;
		//std::string m_PrefabName;

		DirectX::XMMATRIX Origin;

		NodeComponent()
			:
			ParentEntity(UUID::Null),
			RootEntity(UUID::Null),
			NodeName(""),
			//m_PrefabName(""),
			Origin(DirectX::XMMatrixIdentity())
		{};

		NodeComponent(const NodeComponent& other)
		{
			Children = other.Children;
			ParentEntity = other.ParentEntity;
			RootEntity = other.RootEntity;
			NodeName = other.NodeName;
			Origin = other.Origin;
		}
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

		//TEMP
		int ClassIndex = 0;

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

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents = ComponentGroup<TransformComponent, CameraComponent, MeshRendererComponent,
										 RootNodeTag, NodeComponent, LightComponent,
										 ScriptComponent, NativeScriptComponent>;
}