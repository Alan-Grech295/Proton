#pragma once

#include "Proton/Window.h"
#include "Proton/Log.h"
#include "WindowsGraphics.h"
#include <memory>

//Temp includes
#include "Proton/Timer.h"

namespace Proton
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props, HINSTANCE hInstance);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.width; }
		inline unsigned int GetHeight() const override { return m_Data.height; }

		//Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.eventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		void SetTitle(const std::string& title) override;

		WindowsGraphics& Gfx();

		static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT WINAPI HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT WINAPI HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	private:
		virtual void Init(const WindowProperties& props, HINSTANCE hInstance);
		virtual void Shutdown();
	private:
		struct WindowData
		{
			std::string title;
			unsigned int width;
			unsigned int height;
			bool vSync;
			EventCallbackFn eventCallback;
		};

		WindowData m_Data;

		HWND m_HWnd;
		HINSTANCE m_HInstance;
		bool receivingMouseInput;
		std::unique_ptr<WindowsGraphics> pGfx;

		//Temp variables
		Timer timer;
	};
}

#define GET_ERROR(hr) char* msgBuf = nullptr;\
					     DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | \
												   FORMAT_MESSAGE_FROM_SYSTEM | \
												   FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, \
												   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&msgBuf), 0, nullptr);\
						 if(msgLen == 0)\
						 {\
							 PT_CORE_ERROR("Unidentified error code!");\
						 }\
						 std::string errorString = msgBuf;\
						 LocalFree(msgBuf);\
						 PT_CORE_ERROR("Error at line {0} in file {1} \n\t\t[Error Code] {2}\n\t\t[Description] {3}", __LINE__, __FILE__, hr, errorString);

#define LAST_ERROR() GET_ERROR(GetLastError());