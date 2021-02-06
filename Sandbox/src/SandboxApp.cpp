#include <Proton.h>

#include "imgui\imgui.h"

class ExampleLayer : public Proton::Layer
{
public:
	ExampleLayer()
		: Layer("Example"),
		m_Camera(Proton::Application::Get().GetWindow().GetWidth(), Proton::Application::Get().GetWindow().GetHeight(), 0.5f, 1000.0f, Proton::Camera::ProjectionMode::Perspective)
	{
		std::mt19937 rng{ std::random_device{}() };
		std::uniform_int_distribution<int> sdist{ 0,1 };
		std::uniform_real_distribution<float> adist{ 0.0f,Proton::PI * 2.0f };
		std::uniform_real_distribution<float> ddist{ 0.0f,Proton::PI * 0.5f };
		std::uniform_real_distribution<float> odist{ 0.0f,Proton::PI * 0.08f };
		std::uniform_real_distribution<float> rdist{ 6.0f,20.0f };
		std::uniform_real_distribution<float> bdist{ 0.4f,3.0f };
		std::uniform_real_distribution<float> cdist{ 0.0f, 1.0f };

		std::uniform_int_distribution<int> mdist{ 0, nDrawables - 100 };

		int nBoxes = mdist(rng) + 100;
		int nModels = nDrawables - nBoxes;

		boxes.resize(nBoxes);

		for (int i = 0; i < nBoxes; i++)
		{
			boxes[i] = std::make_unique<Proton::Box>(rng, adist, ddist, odist, rdist, bdist, DirectX::XMFLOAT3(cdist(rng), cdist(rng), cdist(rng)));
		}

		models.resize(nModels);

		for (int i = 0; i < nModels; i++)
		{
			models[i] = std::make_unique<Proton::AssimpTest>(rng, adist, ddist, odist, rdist, DirectX::XMFLOAT3(cdist(rng), cdist(rng), cdist(rng)));
		}

		light = std::make_unique<Proton::PointLight>(0.5f);
	}

	void OnUpdate(Proton::TimeStep ts) override
	{
		PT_TRACE("Delta Time: {0}s, {1}ms", ts.GetSeconds(), ts.GetMilliseconds());
		float speed = ts * 15.0f;

		if (Proton::Input::IsKeyPressed(PT_KEY_D))
			m_CameraPos.x += speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_A))
			m_CameraPos.x -= speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_E))
			m_CameraPos.y += speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_Q))
			m_CameraPos.y -= speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_W))
			m_CameraPos.z += speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_S))
			m_CameraPos.z -= speed;

		m_Camera.SetPosition(m_CameraPos);
		Proton::Renderer::BeginScene(m_Camera);

		light->SetLightData();

		for (int i = 0; i < boxes.size(); i++)
		{
			boxes[i]->Update(ts.GetSeconds());

			boxes[i]->m_VertShader->Bind();
			boxes[i]->m_PixelShader->Bind();
			boxes[i]->m_TransformCBuf->Bind();
			boxes[i]->m_MaterialCBuf->Bind();

			Proton::Renderer::Submit(boxes[i]->m_VertBuffer.get(), boxes[i]->m_IndexBuffer.get());
		}

		for (int i = 0; i < models.size(); i++)
		{
			models[i]->Update(ts.GetSeconds());

			models[i]->m_VertShader->Bind();
			models[i]->m_PixelShader->Bind();
			models[i]->m_TransformCBuf->Bind();
			models[i]->m_MaterialCBuf->Bind();

			Proton::Renderer::Submit(models[i]->m_VertBuffer.get(), models[i]->m_IndexBuffer.get());
		}

		//Renderer::EndScene();
		light->mesh.Bind();

		Proton::Renderer::Submit(light->mesh.m_VertBuffer.get(), light->mesh.m_IndexBuffer.get());
	}

	void OnImGuiRender() override
	{
		if (ImGui::Begin("Debug Data"))
		{
			ImGui::Text("Application Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		}

		ImGui::End();
	}

	void OnAttach() override
	{
		PT_TRACE("Attached example layer");
	}

	void OnEvent(Proton::Event& event) override
	{
		if (event.IsEventType(Proton::EventType::KeyPressed))
		{
			Proton::KeyPressedEvent& e = (Proton::KeyPressedEvent&) event;
			//PT_TRACE("{0}", (char)e.GetKeyCode());
		}
	}
private:
	std::vector<std::unique_ptr<Proton::Box>> boxes;
	std::vector<std::unique_ptr<Proton::AssimpTest>> models;
	static constexpr size_t nDrawables = 150;
	Proton::Camera m_Camera;
	std::unique_ptr<Proton::PointLight> light;

	DirectX::XMFLOAT3 m_CameraPos{ 0, 0, -20 };
};

class Sandbox : public Proton::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}

	~Sandbox()
	{

	}
};

Proton::Application* Proton::CreateApplication()
{
	return new Sandbox();
}