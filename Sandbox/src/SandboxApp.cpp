#include <Proton.h>

class Sandbox : public Proton::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}
};

Proton::Application* Proton::CreateApplication()
{
	return new Sandbox();
}