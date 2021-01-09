#pragma once

#ifdef PT_PLATFORM_WINDOWS

extern Proton::Application* Proton::CreateApplication();

int main(int argc, char** argv)
{
	Proton::Log::Init();

	auto app = Proton::CreateApplication();
	app->Run();
	delete app;
}

#endif