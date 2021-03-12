#pragma once

#ifdef PT_PLATFORM_WINDOWS
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

extern Proton::Application* Proton::CreateApplication();

//Win32 Entry point
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//Initializing console as a Win32 app does not open a console
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;

	Proton::Log::Init();

	PT_PROFILE_BEGIN_SESSION("Startup", "ProtonProfile-Startup.json");
	auto app = Proton::CreateApplication();
	PT_PROFILE_END_SESSION();

	PT_PROFILE_BEGIN_SESSION("Runtime", "ProtonProfile-Runtime.json");
	app->Run();
	PT_PROFILE_END_SESSION();

	PT_PROFILE_BEGIN_SESSION("Shutdown", "ProtonProfile-Shutdown.json");
	PT_CORE_TRACE("Close");
	delete app;
	PT_PROFILE_END_SESSION();

	return 0;
}

#endif