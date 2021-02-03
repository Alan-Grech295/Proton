#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Events/KeyEvent.h"
#include "Window.h"
#include "Timer.h"
#include "LayerStack.h"
#include "Renderer\Camera.h"
#include "PointLight.h"

namespace Proton
{
	class PROTON_API Application
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
		//std::unique_ptr<class Box> box;
		Timer timer;
		LayerStack m_LayerStack;

		std::vector<std::unique_ptr<class Box>> boxes;
		static constexpr size_t nDrawables = 150;
	public:
		Camera m_Camera;
		std::unique_ptr<PointLight> light;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

