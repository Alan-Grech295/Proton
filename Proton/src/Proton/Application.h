#pragma once

#include "Core.h"

#include "Window.h"
#include "LayerStack.h"
#include "ImGui\ImGuiLayer.h"

#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"

#include "Timer.h"
#include "Renderer\Camera.h"
#include "PointLight.h"

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

		static inline Application& Get() { return *s_Instance; }

		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		bool OnAppRender(AppRenderEvent& e);
	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		Timer timer;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
		__int64 m_AppStartTime = 0.0f;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

