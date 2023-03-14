#include "ptpch.h"
#include "ImGuiLayer.h"
#include "imgui.h"
#include "examples\imgui_impl_win32.h"
#include "examples\imgui_impl_dx11.h"
#include "Proton\Core\Application.h"
#include "Proton\Core\Log.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"
#include "Proton\Renderer\RenderCommand.h"

#include "ImGuizmo.h"

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

		io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Bold.ttf", 18.0f);
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/opensans/OpenSans-Regular.ttf", 18.0f);

		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		SetDarkThemeColors();

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

		ImGuizmo::BeginFrame();
	}

	void ImGuiLayer::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();

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

	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.0078f, 0.0471f, 0.098f, 1.0f };

		//Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.051f, 0.098f, 0.149f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.071f, 0.118f, 0.169f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.111f, 0.158f, 0.209f, 1.0f };

		//Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.0706f, 0.1059f, 0.1333f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3106f, 0.3459f, 0.3733f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.2306f, 0.2359f, 0.2333f, 1.0f };

		//Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.0667f, 0.1059f, 0.1373f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.0467f, 0.0859f, 0.1173f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.0267f, 0.0659f, 0.0973f, 1.0f };

		//Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.0606f, 0.088f, 0.1037f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.1006f, 0.138f, 0.1537f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.0906f, 0.118f, 0.1337f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.0306f, 0.058f, 0.0837f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.0706f, 0.098f, 0.1137f, 1.0f };

		//Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.0078f, 0.0196f, 0.0314f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.0578f, 0.0696f, 0.0814f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.0078f, 0.0196f, 0.0314f, 1.0f };
	}
}