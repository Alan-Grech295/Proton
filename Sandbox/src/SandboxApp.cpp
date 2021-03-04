#include <Proton.h>

#include "imgui\imgui.h"

class ExampleLayer : public Proton::Layer
{
public:
	ExampleLayer()
		: Layer("Example"),
		m_Camera(Proton::Application::Get().GetWindow().GetWidth(), Proton::Application::Get().GetWindow().GetHeight(), 0.5f, 1000.0f, Proton::Camera::ProjectionMode::Perspective)
	{
		Proton::Application::Get().GetWindow().ShowCursor();
		light = std::make_shared<Proton::PointLight>(0.5f);
	}

	void OnUpdate(Proton::TimeStep ts) override
	{
		float speed = ts * cameraSpeed;

		DirectX::XMFLOAT3 localMove = { 0, 0, 0 };

		if (Proton::Input::IsKeyPressed(PT_KEY_D))
			localMove.x += speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_A))
			localMove.x -= speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_E))
			localMove.y += speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_Q))
			localMove.y -= speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_W))
			localMove.z += speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_S))
			localMove.z -= speed;

		if (Proton::Input::IsKeyPressed(PT_KEY_ESCAPE))
		{
			if (cursor)
			{
				Proton::Application::Get().GetWindow().HideCursor();
				Proton::Application::Get().GetWindow().DisableImGuiCursor();
				Proton::Application::Get().GetWindow().ConfineCursor();
			}
			else
			{
				Proton::Application::Get().GetWindow().ShowCursor();
				Proton::Application::Get().GetWindow().EnableImGuiCursor();
				Proton::Application::Get().GetWindow().FreeCursor();
			}
		}

		if (Proton::Input::IsKeyReleased(PT_KEY_ESCAPE))
		{
			cursor = !cursor;
		}

		if (Proton::Input::IsKeyReleased(PT_KEY_SPACE))
		{
			enableCursor = !enableCursor;
		}

		DirectX::XMStoreFloat3(&localMove, DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&localMove), m_Camera.GetRotationMatrix()));

		m_CameraPos.x += localMove.x;
		m_CameraPos.y += localMove.y;
		m_CameraPos.z += localMove.z;
		
		DirectX::XMFLOAT3 newRot = m_Camera.GetRotation();

		if (enableCursor)
		{
			newRot.x += rotationSpeed * Proton::Input::GetMouseDeltaY() * ts;
			newRot.y += rotationSpeed * Proton::Input::GetMouseDeltaX() * ts;
		}
		
		m_Camera.SetRotation(newRot);
		m_Camera.SetPosition(m_CameraPos);
		Proton::Renderer::BeginScene(m_Camera);

		light->SetLightData();

		Proton::Renderer::Submit(goblin);

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

		light->CreateControlWindow();

		goblin.ShowWindow();

		//nano.ShowWindow("Model 1");

		//nano2.ShowWindow("Model 2");

		//brickWall.ShowWindow("Brick Wall");
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
		}
	}
private:
	static constexpr size_t nDrawables = 150;
	Proton::Camera m_Camera;
	Proton::Ref<Proton::PointLight> light;
	Proton::Model goblin{ "C:\\Dev\\Proton\\Proton\\Models\\Goblin\\GoblinX.obj" };
	//Proton::Model nano{ "C:\\Dev\\Proton\\Proton\\Models\\nano_textured\\nanosuit.obj" };

	DirectX::XMFLOAT3 m_CameraPos{ 0, 0, -20 };
	float cameraSpeed = 15.0f;
	float rotationSpeed = 1.5f;

	bool enableCursor = true;
	bool cursor = true;

	struct
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	} m_Transform;
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