#include "ptpch.h"
#include "PointLight.h"
#include <imgui.h>
#include "Renderer\Renderer.h"

namespace Proton
{
	PointLight::PointLight(float radius)
		:
		mesh(radius)
	{
		Reset();
		cbuf.reset(PixelConstantBuffer::Create(0, sizeof(cbData), &cbData));
	}

	void PointLight::CreateControlWindow()
	{
		if (ImGui::Begin("Light"))
		{
			ImGui::Text("Position");
			ImGui::SliderFloat("X", &cbData.pos.x, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Y", &cbData.pos.y, -60.0f, 60.0f, "%.1f");
			ImGui::SliderFloat("Z", &cbData.pos.z, -60.0f, 60.0f, "%.1f");

			mesh.SetPos(cbData.pos);

			ImGui::Text("Intensity/Color");
			ImGui::SliderFloat("Intensity", &cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f");
			ImGui::ColorEdit3("Diffuse Color", &cbData.diffuseColor.x);
			ImGui::ColorEdit3("Ambient", &cbData.ambient.x);

			ImGui::Text("Falloff");
			ImGui::SliderFloat("Constant", &cbData.attConst, 0.05f, 10.0f, "%.2f");
			ImGui::SliderFloat("Linear", &cbData.attLin, 0.0001f, 4.0f, "%.4f");
			ImGui::SliderFloat("Quadratic", &cbData.attQuad, 0.0000001f, 10.0f, "%.7f");

			if (ImGui::Button("Reset"))
			{
				Reset();
			}
		}

		ImGui::End();
	}

	void PointLight::Reset()
	{
		cbData = {
			{ 0.0f, 0.0f, 0.0f },
			{0.05f, 0.05f, 0.05f},
			{1.0f, 1.0f, 1.0f},
			2.0f,
			1.0f,
			0.045f,
			0.0075f
		};
	}

	/*void PointLight::Draw(WindowsGraphics& gfx) const
	{
		mesh.SetPos(cbData.pos);
		mesh.Draw(gfx);
	}*/

	void PointLight::SetLightData() const
	{
		auto dataCopy = cbData;
		const auto pos = DirectX::XMLoadFloat3(&cbData.pos);
		DirectX::XMStoreFloat3(&dataCopy.pos, DirectX::XMVector3Transform(pos, Renderer::GetCamera().GetViewMatrix()));
		cbuf->SetData(sizeof(dataCopy), &dataCopy);
		cbuf->Bind();
	}
}