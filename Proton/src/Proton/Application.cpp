#include "ptpch.h"

#include "Application.h"

#include "Proton/Log.h"
#include "Proton/Events/ApplicationEvent.h"
#include <iomanip>

#include "Drawable/Header Files/Box.h"

namespace Proton
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create({"Proton Game Engine", 800, 600}));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		std::mt19937 rng(std::random_device{}());
		std::uniform_real_distribution<float> adist(0.0f, 3.1415f * 2.0f);
		std::uniform_real_distribution<float> ddist(0.0f, 3.1415f * 2.0f);
		std::uniform_real_distribution<float> odist(0.0f, 3.1415f * 0.3f);
		std::uniform_real_distribution<float> rdist(6.0f, 20.0f);

		for (auto i = 0; i < 80; i++)
		{
			boxes.push_back(m_Window->CreateBox(rng, adist, ddist, odist, rdist));
		}

		m_Window->SetProjection(DirectX::XMMatrixPerspectiveLH(1.0f, ((float)m_Window->GetHeight() / m_Window->GetWidth()), 0.5f, 40.0f));
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
			//PT_CORE_TRACE("{0}", e);
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
		/*m_Window->DrawTestCube(-timer.Peek(), 0, 0);

		m_Window->DrawTestCube(timer.Peek(),
			(float)mouseX / m_Window->GetWidth() * 2.0f - 1,
			(float)-mouseY / m_Window->GetHeight() * 2.0f + 1.0f);*/

		auto dt = timer.Mark();
		for (auto& b : boxes)
		{
			b->Update(dt);
			m_Window->Draw(b.get());
		}

		return true;
	}

	bool Application::OnMouseMove(MouseMovedEvent& e)
	{
		mouseX = e.GetX();
		mouseY = e.GetY();
		return true;
	}
}
