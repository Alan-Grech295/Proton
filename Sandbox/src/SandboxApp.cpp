#include <Proton.h>

class Sandbox : public Proton::Application
{
public:
	Sandbox()
	{
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