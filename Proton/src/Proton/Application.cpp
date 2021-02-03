#include "ptpch.h"

#include "Application.h"

#include "Proton/Log.h"
#include "Proton/Events/ApplicationEvent.h"
#include <iomanip>
#include "Proton/AssimpTest.h"
#include "Proton/Box.h"
#include <memory>
#include <algorithm>
#include "Proton/Math.h"
#include "imgui.h"
#include "Platform/DirectX 11/imgui_impl_dx11.h"
#include "Platform/DirectX 11/imgui_impl_win32.h"
#include "Input.h"
#include "Renderer\Buffer.h"
#include "Renderer\RenderCommand.h"
#include "Renderer\Renderer.h"
#include "Box.h"

namespace Proton
{
#define BIND_EVENT_FN(x) std::bind(&x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
		:
		m_Camera(1920, 1080, 0.5f, 1000.0f, Camera::ProjectionMode::Perspective)
	{
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create({"Proton Game Engine", 1920, 1080}));

		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_Window->SetVSync(true);

		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> sdist{ 0,1 };
		std::uniform_real_distribution<float> adist{ 0.0f,PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_real_distribution<float> cdist{ 0.0f, 1.0f };
		
		boxes.resize(nDrawables);

		for (int i = 0; i < nDrawables; i++)
		{
			boxes[i] = std::make_unique<Box>(rng, adist, ddist, odist, rdist, bdist, DirectX::XMFLOAT3(cdist(rng), cdist(rng), cdist(rng)));
		}
		
		light = std::make_unique<PointLight>(0.5f);
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
		RenderCommand::SetClearColor(0.02f, 0.07f, 0.2f);

		while (m_Running)
		{
			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}

			RenderCommand::Clear();

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
		Renderer::BeginScene(m_Camera);
		auto dt = timer.Mark();

		light->SetLightData();
		
		for (int i = 0; i < nDrawables; i++)
		{
			boxes[i]->Update(dt);

			boxes[i]->m_VertShader->Bind();
			boxes[i]->m_PixelShader->Bind();
			boxes[i]->m_TransformCBuf->Bind();
			boxes[i]->m_MaterialCBuf->Bind();

			Renderer::Submit(boxes[i]->m_VertBuffer.get(), boxes[i]->m_IndexBuffer.get());
		}
		
		//Renderer::EndScene();
		light->mesh.Bind();

		Renderer::Submit(light->mesh.m_VertBuffer.get(), light->mesh.m_IndexBuffer.get());
		
		return true;
	}
}
