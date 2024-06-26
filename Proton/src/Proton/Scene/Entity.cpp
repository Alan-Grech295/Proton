#include "ptpch.h"
#include "Entity.h"
#include "Scene.h"
//#include "Components.h"

namespace Proton
{
    Entity Entity::Null = Entity{ entt::null, nullptr };
    Entity::Entity(entt::entity entityHandler, Scene* scene)
        :
        m_EntityHandle(entityHandler),
        m_Scene(scene)
    {
    }

    void Entity::SetParent(UUID parentID, int pos)
    {
        std::string name = GetComponent<TagComponent>().Tag;
        UUID myUUID = GetUUID();

        if (parentID != UUID::Null)
        {
            NodeComponent& nodeComponent = GetComponent<NodeComponent>();
            if (nodeComponent.ParentEntity != UUID::Null && nodeComponent.ParentEntity == parentID && pos == -1)
                return;

            NodeComponent& parentNodeComponent = m_Scene->GetEntityByUUID(parentID).GetComponent<NodeComponent>();

            if (HasComponent<RootNodeTag>())
                RemoveComponent<RootNodeTag>();

            int childPos = INT32_MAX;

            if (nodeComponent.ParentEntity != UUID::Null)
            {
                NodeComponent& pastParentNodeComponent = m_Scene->GetEntityByUUID(nodeComponent.ParentEntity).GetComponent<NodeComponent>();
                for (int i = 0; i < pastParentNodeComponent.Children.size(); i++)
                {
                    if (pastParentNodeComponent.Children[i] == myUUID)
                    {
                        childPos = i;
                        pastParentNodeComponent.Children.erase(pastParentNodeComponent.Children.begin() + i);
                        break;
                    }
                }
            }

            if (pos > -1 && pos <= parentNodeComponent.Children.size())
            {
                int insertPos = pos;
                if (nodeComponent.ParentEntity == parentID && childPos < pos)
                    insertPos--;

                parentNodeComponent.Children.insert(parentNodeComponent.Children.begin() + insertPos, myUUID);
            }
            else
            {
                parentNodeComponent.Children.push_back(myUUID);
            }

            nodeComponent.ParentEntity = parentID;

            if (m_Scene->GetEntityByUUID(parentID).HasComponent<RootNodeTag>())
                nodeComponent.RootEntity = parentID;
            else
                nodeComponent.RootEntity = parentNodeComponent.RootEntity;
        }
        else
        {
            if (!HasComponent<RootNodeTag>())
                AddComponent<RootNodeTag>();

            NodeComponent& nodeComponent = GetComponent<NodeComponent>();

            if (nodeComponent.ParentEntity != UUID::Null)
            {
                NodeComponent& parentNodeComponent = m_Scene->GetEntityByUUID(nodeComponent.ParentEntity).GetComponent<NodeComponent>();
                for (int i = 0; i < parentNodeComponent.Children.size(); i++)
                {
                    if (parentNodeComponent.Children[i] == myUUID)
                    {
                        parentNodeComponent.Children.erase(parentNodeComponent.Children.begin() + i);
                        break;
                    }
                }
            }

            nodeComponent.ParentEntity = UUID::Null;
            nodeComponent.RootEntity = myUUID;
        }
    }

    Entity Entity::GetParent() const
    {
        UUID parentUUID = GetComponent<NodeComponent>().ParentEntity;
        if (parentUUID == UUID::Null) return Entity::Null;

        return m_Scene->GetEntityByUUID(parentUUID);
    }

    bool Entity::HasParent(Entity e) const
    {
        if (*this == e) return false;
        if (e == Entity::Null) return false;

        Entity curEntity = *this;
        while (curEntity != Entity::Null)
        {
            curEntity = curEntity.GetParent();
            if (curEntity == e) return true;
        }

        return false;
    }

    bool Entity::HasChild(Entity e) const
    {
        return e.HasParent(*this);
    }

    DirectX::XMMATRIX Entity::LocalTransform()
    {
        if (!HasComponent<TransformComponent>()) return DirectX::XMMatrixIdentity();
        if (!HasComponent<NodeComponent>()) return DirectX::XMMatrixIdentity();

        return GetComponent<TransformComponent>().GetLocalTransformMatrix() *
               GetComponent<NodeComponent>().Origin;
    }

    DirectX::XMMATRIX Entity::LocalToWorld()
    {
        // Return zero matrix instead?
        if (!HasComponent<TransformComponent>()) return DirectX::XMMatrixIdentity();
        if (!HasComponent<NodeComponent>()) return DirectX::XMMatrixIdentity();

        // TODO: Find way to not store transforms
        std::vector<DirectX::XMMATRIX> transforms;
        Entity parentEntity = GetParent();

        while (parentEntity != Entity::Null)
        {
            transforms.push_back(parentEntity.LocalTransform());
            //worldTransform = worldTransform * parentEntity.LocalTransform();

            parentEntity = parentEntity.GetParent();
        }

        DirectX::XMMATRIX worldTransform = LocalTransform();

        for (auto it = transforms.rbegin(); it != transforms.rend(); ++it)
        {
            worldTransform *= *it;
        }

        return worldTransform;
    }

    DirectX::XMMATRIX Entity::WorldToLocal(DirectX::FXMMATRIX worldTransform)
    {
        if (!HasComponent<TransformComponent>()) return worldTransform;

        // TODO: Find way to not store transforms
        std::vector<DirectX::XMMATRIX> transforms;
        Entity parentEntity = GetParent();

        while (parentEntity != Entity::Null)
        {
            transforms.push_back(DirectX::XMMatrixInverse(nullptr,
                parentEntity.LocalTransform()));

            parentEntity = parentEntity.GetParent();
        }

        DirectX::XMMATRIX localTransform = worldTransform;

        for (auto it = transforms.rbegin(); it != transforms.rend(); ++it)
        {
            localTransform *= *it;
        }

        return localTransform;
    }

    const UUID Entity::GetUUID()
    {
        return GetComponent<IDComponent>().ID;
    }

    const std::string& Entity::GetName()
    {
        return GetComponent<TagComponent>().Tag;
    }

    template<typename T>
    void Entity::AddScript()
    {
        AddComponent<NativeScriptComponent>().Bind<T>();
    }

    template<typename T>
    void Entity::OnComponentAdded(T& component)
    {
        static_assert(sizeof(T) != 0, "Component cannot be of size 0");
    }

    template<>
    void Entity::OnComponentAdded<IDComponent>(IDComponent& component){}

    template<>
    void Entity::OnComponentAdded<TransformComponent>(TransformComponent& component) {}

    template<>
    void Entity::OnComponentAdded<TagComponent>(TagComponent& component) {}

    template<>
    void Entity::OnComponentAdded<CameraComponent>(CameraComponent& component) 
    {
        component.Camera.SetViewportSize(m_Scene->GetViewportWidth(), m_Scene->GetViewportHeight());
    }

    template<>
    void Entity::OnComponentAdded<MeshRendererComponent>(MeshRendererComponent& component) {}

    template<>
    void Entity::OnComponentAdded<RootNodeTag>(RootNodeTag& component) {}

    template<>
    void Entity::OnComponentAdded<NodeComponent>(NodeComponent& component) {}

    template<>
    void Entity::OnComponentAdded<LightComponent>(LightComponent& component) {}

    template<>
    void Entity::OnComponentAdded<ScriptComponent>(ScriptComponent& component) {}
    
    template<>
    void Entity::OnComponentAdded<NativeScriptComponent>(NativeScriptComponent& component){}
}