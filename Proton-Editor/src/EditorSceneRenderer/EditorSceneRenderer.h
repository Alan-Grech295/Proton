#pragma once
#include "Proton/Renderer/SceneRenderer.h"
#include "Proton/Renderer/Render Queue/Pass.h"
#include "Proton/Scene/Entity.h"

#include <unordered_map>

namespace Proton
{
    class EditorSceneRenderer : public SceneRenderer
    {
    public:
        EditorSceneRenderer(const Ref<Scene> scene, const FramebufferDescription& framebufferDesc);

        virtual void Render(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix) override;

        void RenderPickOutline();

        void SetSelectedEntity(Entity e) { selectedEntity = e; }
    protected: 
        virtual void SubmitMesh(Entity entity, const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf) override;
    private:
        Ref<Material> GetPickMaterial(Entity e);
    private:
        std::unordered_map<Entity, Ref<Material>> pickMaterials;
        Pass pickOutlineMask;
        Pass pickOutline;
        Ref<Material> PickOutlineMaterial;
        Ref<Material> PickOutlineMaskMaterial;

        Entity selectedEntity = Entity::Null;
    };
}