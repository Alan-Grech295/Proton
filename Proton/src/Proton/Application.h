#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "Window.h"
#include "Timer.h"
//#include "Drawable/Box.h"

namespace Proton
{
	class PROTON_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
	private:
		bool OnWindowClose(WindowCloseEvent& e);

		bool OnAppRender(AppRenderEvent& e);

		bool OnMouseMove(MouseMovedEvent& e);

		bool m_Running = true;
		std::unique_ptr<Window> m_Window;
		Timer timer;
		float mouseX, mouseY;

		std::vector<std::unique_ptr<class Drawable>> drawables;
		static constexpr size_t nDrawables = 180;
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

