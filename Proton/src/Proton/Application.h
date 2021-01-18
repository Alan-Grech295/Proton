#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Window.h"
#include "Timer.h"
#include "LayerStack.h"
#include "Platform/Windows/Camera.h"

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

		bool OnMouseMove(MouseMovedEvent& e);

	private:
		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		Timer timer;
		float mouseX, mouseY;
		LayerStack m_LayerStack;

		std::vector<std::unique_ptr<class Drawable>> drawables;
		static constexpr size_t nDrawables = 180;

		Camera camera;
	private:
		static Application* s_Instance;
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

