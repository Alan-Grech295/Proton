#pragma once

#ifdef PT_PLATFORM_WINDOWS
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include "Platform/DirectX 11/Debugging/Exceptions.h"
#include <iostream>
//TEMP
#include "Proton/Core/Core.h"

extern Proton::Application* Proton::CreateApplication(ApplicationCommandLineArgs args);

#define CATCH_EXCEPTIONS(func)	try \
								{ \
									func; \
								} \
								catch (const Proton::Exception& e) \
								{ \
									MessageBoxA(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION); \
								} \
								catch (const std::exception& e) \
								{ \
									MessageBoxA(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION); \
								} \
								catch (...) \
								{ \
									MessageBoxA(nullptr, "No details available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION); \
								}

#define LOG_CATCH_EXCEPTIONS(func)	try \
								{ \
									func; \
								} \
								catch (const Proton::Exception& e) \
								{ \
									PT_CORE_ERROR("[{}] {}", e.GetType(), e.what()); __debugbreak();\
								} \
								catch (const std::exception& e) \
								{ \
									PT_CORE_ERROR("[Standard Exception] {}", e.what()); __debugbreak();\
								} \
								catch (...) \
								{ \
									PT_CORE_ERROR("[No details available] Unknown Exception"); __debugbreak();\
								}\

//Win32 Entry point
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	//TODO: Investigate memory leaks
	//_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) & ~_CRTDBG_ALLOC_MEM_DF);
	//_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG | _CRTDBG_LEAK_CHECK_DF);

	//#include "crtdbg.h"
	//
	//#ifdef _DEBUG
	//#define DEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
	//#else
	//#define DEBUG_NEW
	//#endif
	//
	//#ifdef _DEBUG
	//#define new DEBUG_NEW
	//#endif

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
	Proton::Application* app;
	LOG_CATCH_EXCEPTIONS(app = Proton::CreateApplication({ __argc, __argv }));

	PT_PROFILE_END_SESSION();

	PT_PROFILE_BEGIN_SESSION("Runtime", "ProtonProfile-Runtime.json");
	CATCH_EXCEPTIONS(app->Run());
	PT_PROFILE_END_SESSION();

	PT_PROFILE_BEGIN_SESSION("Shutdown", "ProtonProfile-Shutdown.json");
	CATCH_EXCEPTIONS(delete app);
	PT_PROFILE_END_SESSION();

	//_CrtDumpMemoryLeaks();

	return 0;
}

#endif