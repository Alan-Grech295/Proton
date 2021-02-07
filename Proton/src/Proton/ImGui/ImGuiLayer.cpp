#include "ptpch.h"
#include "ImGuiLayer.h"
#include "imgui.h"
#include "examples\imgui_impl_win32.h"
#include "examples\imgui_impl_dx11.h"
#include "Proton/Application.h"
#include "Proton\Log.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"
#include "Proton\Renderer\RenderCommand.h"

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
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application::Get().GetWindow().InitImGui();

		PT_CORE_INFO("Attached layer {0}", m_DebugName);
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnImGuiRender()
	{
		//static bool show = true;
		//ImGui::ShowDemoWindow(&show);
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();

		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			DirectXRendererAPI* api = (DirectXRendererAPI*)RenderCommand::GetRendererAPI();
			ID3D11RenderTargetView* backupTarget = api->GetRenderTarget();

			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();

			api->SetRenderTarget(backupTarget);
		}
	}
}