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
        EditorSceneRenderer(const Ref<Scene> scene, const FramebufferDescription& framebufferDesc)
            : SceneRenderer(scene, framebufferDesc)
        {}

        virtual void Render(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix) override;

    protected: 
        virtual void SubmitMesh(Entity entity, const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf) override;
    private:
        Ref<Material> GetPickMaterial(Entity e);
    private:
        Pass pickPass;
        std::unordered_map<Entity, Ref<Material>> pickMaterials;
    };
}