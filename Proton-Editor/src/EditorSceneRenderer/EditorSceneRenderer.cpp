#include "EditorSceneRenderer.h"

namespace Proton
{
    void EditorSceneRenderer::Render(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix)
    {
        pickPass.Clear();

        SceneRenderer::Render(viewMatrix, projMatrix);

        m_FrameBuffer->ClearDepth();

        Renderer::RenderPass(pickPass);
    }

    void EditorSceneRenderer::SubmitMesh(Entity entity, const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf)
    {
        SceneRenderer::SubmitMesh(entity, mesh, materials, vertTransformBuf, pixTransformBuf);

        pickPass.AddJob({ mesh, vertTransformBuf, pixTransformBuf, GetPickMaterial(entity)});
    }

    static Ref<Material> CreatePickMaterial()
    {
        namespace dx = DirectX;

        // Single pass
        Ref<Material> material = CreateRef<Material>();

        std::string pixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PickerPS.cso";
        std::string vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PickerVS.cso";

        material->m_PixelShader = PixelShader::Create(pixShaderPath);
        material->m_VertexShader = VertexShader::Create(vertShaderPath);

        DCB::RawLayout layout;
        layout.Add(DCB::Type::UInt, "EntityID");

        Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));

        material->AddBindable(pcb);

        return material;
    }

    Ref<Material> EditorSceneRenderer::GetPickMaterial(Entity e)
    {
        if (pickMaterials.contains(e))
        {
            return pickMaterials[e];
        }

        Ref<Material> pickMaterial = CreatePickMaterial();
        (*pickMaterial->GetBindable<PixelConstantBuffer>())["EntityID"] = (uint32_t)e;
        pickMaterials[e] = pickMaterial;
        return pickMaterial;
    }
}
