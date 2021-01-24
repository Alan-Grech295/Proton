#include <Proton.h>

class ExampleLayer : public Proton::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{}

	void OnUpdate() override
	{
		
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
};

class Sandbox : public Proton::Application
{
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
		PushOverlay(new Proton::ImGuiLayer());
	}

	~Sandbox()
	{

	}
};

Proton::Application* Proton::CreateApplication()
{
	return new Sandbox();
}