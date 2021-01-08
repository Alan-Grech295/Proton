#pragma once

#ifdef PT_PLATFORM_WINDOWS

extern Proton::Application* Proton::CreateApplication();

int main(int argc, char** argv)
{
	Proton::Log::Init();
	PT_CORE_WARN("Initialized Log!");
	int a = 5;
	PT_INFO("Hello, var = {0}", a);

	auto app = Proton::CreateApplication();
	app->Run();
	delete app;
}

#endif