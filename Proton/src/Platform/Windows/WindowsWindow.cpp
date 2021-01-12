#include "ptpch.h"
#include "WindowsWindow.h"
#include "Proton/Events/ApplicationEvent.h"
#include "Proton/Events/MouseEvent.h"
#include "Proton/Events/KeyEvent.h"

namespace Proton
{
	Window* Window::Create(const WindowProperties& props)
	{
		return new WindowsWindow(props, GetModuleHandle(nullptr));
	}

	WindowsWindow::WindowsWindow(const WindowProperties& props, HINSTANCE hInstance)
	{
		Init(props, hInstance);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::OnUpdate()
	{
		//Message handling
		MSG msg;

		GetMessage(&msg, nullptr, 0, 0);

		TranslateMessage(&msg);

		DispatchMessage(&msg);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		m_Data.vSync = enabled;

		//TODO: Add vsync
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.vSync;
	}

	void WindowsWindow::Init(const WindowProperties& props, HINSTANCE hInstance)
	{
		m_Data.title = props.title;
		m_Data.width = props.width;
		m_Data.height = props.height;

		PT_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

		std::wstring tempStr = std::wstring(props.title.begin(), props.title.end());

		const LPCWSTR className = tempStr.c_str();

		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = sizeof(m_Data);
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = nullptr;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = className;
		wc.hIconSm = nullptr;

		RegisterClassEx(&wc);

		HWND hWnd = CreateWindowEx(0, className, className,
			WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
			200, 200, props.width, props.height, nullptr, nullptr,
			hInstance, nullptr);

		SetClassLongPtr(hWnd, 0, (LONG_PTR)&m_Data);

		ShowWindow(hWnd, SW_SHOW);
	}

	void WindowsWindow::Shutdown()
	{
		PostQuitMessage(1);
	}

	LRESULT CALLBACK WindowsWindow::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		WindowData& data = *(WindowData*)GetClassLongPtr(hWnd, 0);

		switch (msg)
		{
		case WM_CLOSE:
			Shutdown();
			WindowCloseEvent event;
			data.eventCallback(event);
			break;
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
