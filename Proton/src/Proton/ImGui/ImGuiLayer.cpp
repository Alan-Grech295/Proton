#include "ptpch.h"
#include "ImGuiLayer.h"
#include "imgui.h"
#include "Platform/DirectX 11/imgui_impl_dx11.h"
#include "Platform/DirectX 11/imgui_impl_win32.h"
#include "Proton/Application.h"
#include "Proton\Log.h"

namespace Proton
{
	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{

	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		Application::Get().GetWindow().InitImGui();

		PT_CORE_INFO("Attached layer {0}", m_DebugName);
	}

	void ImGuiLayer::OnDetach()
	{

	}

	void ImGuiLayer::OnUpdate()
	{
		
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		if (event.IsEventType(EventType::AppRender))
		{
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			if (ImGui::Begin("Debug Data"))
			{
				ImGui::Text("Application Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			}

			ImGui::End();

			Application::Get().light->CreateControlWindow();

			ImGui::Render();
			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		}
	}
}