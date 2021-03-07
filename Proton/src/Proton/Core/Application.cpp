#include "ptpch.h"

#include "Application.h"

#include "Proton\Core\Log.h"
#include "Proton/Events/ApplicationEvent.h"
#include <iomanip>
#include <memory>
#include <algorithm>
#include "Proton\Core\Math.h"
#include "imgui.h"
#include "Input.h"

#include "Proton\Core\Base.h"

#include "Proton\Renderer\Renderer.h"

#include "Proton\Debug\ProfileLayer.h"

namespace Proton
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		m_Window = Scope<Window>(Window::Create({"Proton Game Engine", 1280, 720}));

		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		#if PT_PROFILE
		PushOverlay(new ProfileLayer());
		#endif
	}

	Application::~Application()
	{
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClose));

		dispatcher.Dispatch<AppRenderEvent>(BIND_EVENT_FN(Application::OnAppRender));

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
		RenderCommand::SetClearColor(0.02f, 0.07f, 0.2f);

		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		m_AppStartTime = li.QuadPart;

		while (m_Running)
		{
			RenderCommand::Clear();

			QueryPerformanceFrequency(&li);
			double pcFreq = li.QuadPart;
			QueryPerformanceCounter(&li);

			float time = (li.QuadPart - m_AppStartTime) / pcFreq;		//Platform::GetTime()

			TimeStep timeStep = time - m_LastFrameTime;

			m_LastFrameTime = time;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timeStep);

			m_ImGuiLayer->Begin();

			for (Layer* layer : m_LayerStack)
				layer->OnImGuiRender();

			m_ImGuiLayer->End();

			m_Window->OnUpdate(timeStep);
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnAppRender(AppRenderEvent& e)
	{
		
		
		return true;
	}
}
