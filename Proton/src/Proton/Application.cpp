#include "ptpch.h"

#include "Application.h"

#include "Proton/Log.h"
#include "Proton/Events/ApplicationEvent.h"
#include <iomanip>
#include "Proton/Melon.h"
#include "Proton/Pyramid.h"
#include "Proton/Box.h"
#include "Proton/Sheet.h"
#include <memory>
#include <algorithm>
#include "Proton/Math.h"
#include "imgui.h"
#include "Platform/DirectX 11/imgui_impl_dx11.h"
#include "Platform/DirectX 11/imgui_impl_win32.h"

namespace Proton
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create({"Proton Game Engine", 1280, 720}));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		class Factory
		{
		public:
			Factory(Window* window)
				:
				wnd(window)
			{}
			std::unique_ptr<Drawable> operator()()
			{
				switch (typedist(rng))
				{
				case 0:
					return wnd->CreatePyramid(rng, adist, ddist,
						odist, rdist);
				case 1:
					return wnd->CreateBox(
						rng, adist, ddist,
						odist, rdist, bdist
						);
				case 2:
					return wnd->CreateMelon(
						rng, adist, ddist,
						odist, rdist, longdist, latdist
						);
				case 3:
					return wnd->CreateSheet(
						rng, adist, ddist,
						odist, rdist
						);
				default:
					assert(false && "bad drawable type in factory");
					return {};
				}
			}
		private:
			Window* wnd;
			std::mt19937 rng{ std::random_device{}() };
			std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
			std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
			std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
			std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
			std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
			std::uniform_int_distribution<int> latdist{ 5,20 };
			std::uniform_int_distribution<int> longdist{ 10,40 };
			std::uniform_int_distribution<int> typedist{ 0,3 };
		};

		drawables.reserve(nDrawables);
		std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ m_Window.get() });

		m_Window->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, ((float)m_Window->GetHeight() / m_Window->GetWidth()), 0.5f, 1000.0f));
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		dispatcher.Dispatch<AppRenderEvent>(BIND_EVENT_FN(Application::OnAppRender));

		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(Application::OnMouseMove));

		//Only displays events which are not AppRender, as this
		//event is called every frame and clutters up the console
		if (!e.IsEventType(EventType::AppRender))
		{
			PT_CORE_TRACE("{0}", e);
		}

		return;

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);

			if (e.Handled())
				break;	
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	void Application::Run()
	{
		while (m_Running)
		{
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnAppRender(AppRenderEvent& e)
	{
		m_Window->SetCamera(camera.GetMatrix());
		auto dt = timer.Mark();

		for (auto& d : drawables)
		{
			d->Update(dt);
			m_Window->Draw(d.get());
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Begin("Debug Data"))
		{
			ImGui::Text("Application Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		ImGui::End();

		camera.CreateControlWindow();

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		return true;
	}

	bool Application::OnMouseMove(MouseMovedEvent& e)
	{
		mouseX = e.GetX();
		mouseY = e.GetY();
		return true;
	}
}
