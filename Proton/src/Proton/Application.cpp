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
#include "Input.h"

namespace Proton
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create({"Proton Game Engine", 1920, 1080}));

		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(false);

		class Factory
		{
		public:
			Factory(Window* window)
				:
				wnd(window)
			{}
			std::unique_ptr<Drawable> operator()()
			{
				return wnd->CreateBox(
					rng, adist, ddist,
					odist, rdist, bdist
				);
			}
		private:
			Window* wnd;
			std::mt19937 rng{ std::random_device{}() };
			std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
			std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
			std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
			std::uniform_real_distribution<float> rdist{ 2.0f,50.0f };
			std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		};

		drawables.reserve(nDrawables);
		std::generate_n(std::back_inserter(drawables), nDrawables, Factory{ m_Window.get() });

		m_Window->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, ((float)m_Window->GetHeight() / m_Window->GetWidth()), 0.5f, 1000.0f));
	
		light = m_Window->CreateLight();
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		dispatcher.Dispatch<AppRenderEvent>(BIND_EVENT_FN(Application::OnAppRender));

		//Only displays events which are not AppRender, as this
		//event is called every frame and clutters up the console
		if (!e.IsEventType(EventType::AppRender))
		{
			//PT_CORE_TRACE("{0}", e);
		}

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
		m_Window->BindLight(light);

		for (auto& d : drawables)
		{
			d->Update(dt);
			m_Window->Draw(d.get());
		}

		m_Window->DrawLight(light);

		return true;
	}
}
