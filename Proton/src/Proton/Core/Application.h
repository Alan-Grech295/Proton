#pragma once

#include "Core.h"

#include "Window.h"
#include "LayerStack.h"
#include "Proton\ImGui\ImGuiLayer.h"

#include "Proton\Events/Event.h"
#include "Proton\Events/ApplicationEvent.h"
#include "Proton\Events/MouseEvent.h"
#include "Proton\Events/KeyEvent.h"

#include "Proton\Renderer\Camera.h"

#include "Proton\Core\TimeStep.h"

namespace Proton
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void Close();

		static inline Application& Get() { return *s_Instance; }

		inline Window& GetWindow() { return *m_Window; }

		void SubmitToMainThread(std::function<void()> function) 
		{ 
			std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);
			m_MainThreadQueue.emplace_back(function); 
		}
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		bool OnAppRender(AppRenderEvent& e);

		void ExecuteMainThreadQueue();
	private:
		bool m_Running = true;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
		__int64 m_AppStartTime = 0;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

