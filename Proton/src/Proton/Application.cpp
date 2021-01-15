#include "ptpch.h"

#include "Application.h"

#include "Proton/Log.h"
#include "Proton/Events/ApplicationEvent.h"
#include <iomanip>

namespace Proton
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create({"Proton Game Engine", 800, 600}));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);
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
	}

	void Application::Run()
	{
		while (m_Running)
		{
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
		m_Window->DrawTestCube(-timer.Peek(), 0, 0);

		m_Window->DrawTestCube(timer.Peek(),
			(float)mouseX / m_Window->GetWidth() * 2.0f - 1,
			(float)-mouseY / m_Window->GetHeight() * 2.0f + 1.0f);

		return true;
	}

	bool Application::OnMouseMove(MouseMovedEvent& e)
	{
		mouseX = e.GetX();
		mouseY = e.GetY();
		return true;
	}
}
