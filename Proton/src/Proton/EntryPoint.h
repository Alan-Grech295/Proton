#pragma once

#ifdef PT_PLATFORM_WINDOWS

extern Proton::Application* Proton::CreateApplication();

int main(int argc, char** argv)
{
	auto app = Proton::CreateApplication();
	app->Run();
	delete app;
}

#endif