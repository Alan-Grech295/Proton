#include "EditorSceneRenderer.h"

namespace Proton
{
    EditorSceneRenderer::EditorSceneRenderer(const Ref<Scene> scene, const FramebufferDescription& framebufferDesc)
        : SceneRenderer(scene, framebufferDesc)
    {
        int pickPassID = Renderer::AddPass("Pick");
        Renderer::AddPreRenderCallback("Pick", [this]() {
            m_FrameBuffer->ClearDepth();
            });

        // Pick outline material
        PickOutlineMaterial = CreateRef<Material>();
        std::string outlinePixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PickerOutlinePS.cso";
        std::string vertShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\FullscreenVS.cso";

        PickOutlineMaterial->m_PixelShader = PixelShader::Create(outlinePixShaderPath);
        PickOutlineMaterial->m_VertexShader = VertexShader::Create(vertShaderPath);

        DCB::RawLayout layout;
        layout.Add(DCB::Type::UInt, "Thickness");
        layout.Add(DCB::Type::Float3, "Color");

        Ref<PixelConstantBuffer> pcb = PixelConstantBuffer::CreateUnique(1, DCB::CookedLayout(std::move(layout)));
        (*pcb)["Thickness"] = 2;
        (*pcb)["Color"] = DirectX::XMFLOAT3(1, 0.6f, 0.2f);

        PickOutlineMaterial->AddBindable(pcb);

        PickOutlineMaterial->AddBindable(Topology::Create(TopologyType::TriangleList));

        PickOutlineMaterial->AddBindable(m_FrameBuffer->GetRenderTexture(2));

        pickOutline.AddJob(CreateRef<Job>(PickOutlineMaterial));

        // Pick outline mask material
        PickOutlineMaskMaterial = CreateRef<Material>();
        std::string maskPixShaderPath = CoreUtils::CORE_PATH_STR + "Proton\\PickerMaskPS.cso";

        PickOutlineMaskMaterial->m_PixelShader = PixelShader::Create(maskPixShaderPath);
        PickOutlineMaskMaterial->m_VertexShader = VertexShader::Create(CoreUtils::CORE_PATH_STR + "Proton\\PickerVS.cso");
    }

    void EditorSceneRenderer::Render(const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projMatrix)
    {
        //pickOutline.Clear();
        pickOutlineMask.Clear();

        SceneRenderer::Render(viewMatrix, projMatrix);
    }

    void EditorSceneRenderer::RenderPickOutline()
    {
        m_FrameBuffer->Clear(2);

        m_FrameBuffer->Bind(false);

        Renderer::RenderPass(pickOutlineMask);

        m_FrameBuffer->BindExclude(2, false);

        PickOutlineMaterial->SetBindable(m_FrameBuffer->GetRenderTexture(2));

        Renderer::RenderPass(pickOutline);
    }

    void EditorSceneRenderer::SubmitMesh(Entity entity, const Mesh* mesh, const std::vector<Ref<Material>>& materials, VertexConstantBuffer* vertTransformBuf, PixelConstantBuffer* pixTransformBuf)
    {
        SceneRenderer::SubmitMesh(entity, mesh, materials, vertTransformBuf, pixTransformBuf);

        Renderer::Submit(mesh, GetPickMaterial(entity), vertTransformBuf, pixTransformBuf);

        if (entity == selectedEntity || entity.HasParent(selectedEntity))
        {
            pickOutlineMask.AddJob(CreateRef<MeshJob>(mesh, vertTransformBuf, pixTransformBuf, PickOutlineMaskMaterial));
        }
    }

    static Ref<Material> CreatePickMaterial()
    {
        namespace dx = DirectX;

        // Single pass
        Ref<Material> material = CreateRef<Material>("Pick");

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
