#include "ptpch.h"
#include <stdlib.h>
#include <crtdbg.h>

#include "Application.h"

#include "Proton\Core\Log.h"
#include "Proton/Events/ApplicationEvent.h"
#include <iomanip>
#include <memory>
#include <algorithm>
#include "Proton\Core\Math.h"
#include "imgui.h"
#include "Input.h"

#include "Proton\Core\Core.h"

#include "Proton\Renderer\Renderer.h"

#include "Proton\Debug\ProfileLayer.h"
#include "Proton/Scripting/ScriptEngine.h"

namespace Proton
{

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
	{
		// Setting working directory
		std::string_view fileLoc = __FILE__;
		size_t pos = fileLoc.find("Proton");
		PT_CORE_ASSERT(pos != -1);
		CoreUtils::CORE_PATH_STR = fileLoc.substr(0, pos + 7);

		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		s_Instance = this;
		m_Window = Scope<Window>(Window::Create({m_Specification.Name, 1280, 720}));

		m_Window->SetEventCallback(PT_BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		#if PT_PROFILE
		PushOverlay(new ProfileLayer());
		#endif

		ScriptEngine::Init();
	}

	Application::~Application()
	{
		m_Window->Close();
		ScriptEngine::Shutdown();
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);

		dispatcher.Dispatch<WindowCloseEvent>(PT_BIND_EVENT_FN(Application::OnWindowClose));

		dispatcher.Dispatch<AppRenderEvent>(PT_BIND_EVENT_FN(Application::OnAppRender));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(e);

			if (e.Handled)
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

	void Application::Close()
	{
		
	}

	void Application::Run()
	{
		RenderCommand::SetClearColor(0.02f, 0.07f, 0.2f);

		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		m_AppStartTime = li.QuadPart;

		while (m_Running)
		{
			QueryPerformanceFrequency(&li);
			double pcFreq = (double)li.QuadPart;
			QueryPerformanceCounter(&li);

			float time = (float)((double)(li.QuadPart - m_AppStartTime) / pcFreq);		//Platform::GetTime()

			TimeStep timeStep = time - m_LastFrameTime;

			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			{
				PT_PROFILE_SCOPE("LayerStack OnUpdate")
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timeStep);
			}

			RenderCommand::BindSwapChain();
			RenderCommand::Clear();

			m_ImGuiLayer->Begin();

			{
				PT_PROFILE_SCOPE("LayerStack OnImGuiRender")
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
			}

			m_ImGuiLayer->End();

			m_Window->OnUpdate(timeStep);
			RenderCommand::Present();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}

	bool Application::OnAppRender(AppRenderEvent& e)
	{
		return false;
	}

	void Application::ExecuteMainThreadQueue()
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue)
			func();

		m_MainThreadQueue.clear();
	}
}
