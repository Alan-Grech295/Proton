#include "ptpch.h"
#include "WindowsWindow.h"
#include "Proton/Events/ApplicationEvent.h"
#include "Proton/Events/MouseEvent.h"
#include "Proton/Events/KeyEvent.h"
#include "Proton/Box.h"
#include "Proton/AssimpTest.h"
//#include "Proton/Drawable.h"
#include "examples\imgui_impl_win32.h"
#include "examples\imgui_impl_dx11.h"
#include "Proton/PointLight.h"
#include "Proton\Renderer\RenderCommand.h"
#include "Platform\DirectX 11\DirectXRendererAPI.h"

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
		input->mouseDeltaX = 0;
		input->mouseDeltaY = 0;

		input->releasedKeyStates.reset();

		//Message handling
		MSG msg;

		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//Dispatches an AppRenderEvent. This is done after the render 
		//but before presenting the frame so that if any post processing
		//effect want to be applied, they will show in the frame
		AppRenderEvent event;
		m_Data.eventCallback(event);

		//Present the frame
		api->ShowFrame();
	}

	void WindowsWindow::ShowCursor() const
	{
		while (::ShowCursor(TRUE) < 0);
	}

	void WindowsWindow::HideCursor() const
	{
		while (::ShowCursor(FALSE) >= 0);
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		m_Data.vSync = enabled;
		api->isVSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.vSync;
	}

	void WindowsWindow::SetTitle(const std::string& title)
	{
		//Store the title as an LPCW string so it can be used
		std::wstring tempStr = std::wstring(title.begin(), title.end());
		const LPCWSTR titleName = tempStr.c_str();

		SetWindowTextW(m_HWnd, titleName);
	}

	void WindowsWindow::InitImGui()
	{
		//Init ImGui Win32 Impl
		ImGui_ImplWin32_Init(m_HWnd);
		ImGui_ImplDX11_Init(api->pDevice.Get(), api->pContext.Get());
		initializedImGui = true;
	}

	void WindowsWindow::Init(const WindowProperties& props, HINSTANCE hInstance)
	{
		input = WindowsInput::s_Instance;

		//Saves any needed data
		m_HInstance = hInstance;
		m_Data.title = props.title;
		m_Data.width = props.width;
		m_Data.height = props.height;

		//Logs window creation
		PT_CORE_INFO("Creating window {0} ({1}, {2})", props.title, props.width, props.height);

		//Store the class name as an LPCW string so it can be used
		std::wstring tempStr = std::wstring(props.title.begin(), props.title.end());
		const LPCWSTR className = tempStr.c_str();

		//Creates the window class
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_OWNDC;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = sizeof(m_Data);
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = nullptr;
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = className;
		wc.hIconSm = nullptr;

		//Registers the class
		RegisterClassEx(&wc);

		//Calculates window size based on client region size
		RECT wr;
		wr.left = 100;
		wr.right = wr.left + props.width;
		wr.top = 100;
		wr.bottom = wr.top + props.height;

		if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
		{
			LAST_ERROR();
		}

		//Creates the window and gets HWND
		m_HWnd = CreateWindow(className, className, 
								 WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
								 CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
								 nullptr, nullptr, hInstance, this);

		//If CreateWindow() fails
		if (m_HWnd == nullptr)
		{
			LAST_ERROR();
		}

		SetCapture(m_HWnd);

		SetClassLongPtr(m_HWnd, 0, (LONG_PTR)&m_Data);

		//Shows the window
		ShowWindow(m_HWnd, SW_SHOWDEFAULT);

		//Create graphics object
		//pGfx = std::make_unique<WindowsGraphics>(m_HWnd, (UINT)m_Data.width, (UINT)m_Data.height);
		api = ((DirectXRendererAPI*)RenderCommand::s_RendererAPI);
		api->Initialize(*this, m_HWnd);

		RAWINPUTDEVICE rid;
		rid.usUsagePage = 0x01;
		rid.usUsage = 0x02;
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;

		if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
		{
			PT_CORE_ERROR("Could not initialized Raw Input Device!");
		}
	}
	
	void WindowsWindow::Shutdown()
	{
		if(initializedImGui)
			ImGui_ImplWin32_Shutdown();

		initializedImGui = false;
		DestroyWindow(m_HWnd);
		PostQuitMessage(1);
	}

	LRESULT CALLBACK WindowsWindow::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (initializedImGui && ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		{
			return true;
		}

		WindowData& data = *(WindowData*)GetClassLongPtr(hWnd, 0);

		switch (msg)
		{
			//Close window message
		case WM_CLOSE:
			{
				Shutdown();
				WindowCloseEvent event;
				data.eventCallback(event);
				return 0;
			}
			//Keyboard messages
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN:
			{
				input->pressedKeyStates[wParam] = true;
				KeyPressedEvent event(wParam, lParam & 0xffff);
				data.eventCallback(event);
				break;
			}
		case WM_SYSKEYUP:
		case WM_KEYUP:
			{
				input->pressedKeyStates[wParam] = false;
				input->releasedKeyStates[wParam] = true;
				KeyReleasedEvent event(wParam);
				data.eventCallback(event);
				break;
			}
		case WM_CHAR:
			{
				//Character input message
				break;
			}
			//Mouse messages
		case WM_LBUTTONDOWN:
			{
				input->mbStates[0] = true;
				const POINTS pt = MAKEPOINTS(lParam);
				MouseButtonPressedEvent event(0, pt.x, pt.y);
				data.eventCallback(event);
			}
			break;
		case WM_LBUTTONUP:
			{
				input->mbStates[0] = false;
				const POINTS pt = MAKEPOINTS(lParam);
				MouseButtonReleasedEvent event(0, pt.x, pt.y);
				data.eventCallback(event);

				if (pt.x < 0 || pt.x >= (SHORT)m_Data.width || pt.y < 0 || pt.y >= (SHORT)m_Data.height)
				{
					ReleaseCapture();
					MouseLeftEvent event(pt.x, pt.y);
					data.eventCallback(event);
					receivingMouseInput = false;
				}
			}
			break;
		case WM_RBUTTONDOWN:
			{
				input->mbStates[1] = true;
				const POINTS pt = MAKEPOINTS(lParam);
				MouseButtonPressedEvent event(1, pt.x, pt.y);
				data.eventCallback(event);
			}
			break;
		case WM_RBUTTONUP:
			{
				input->mbStates[1] = false;
				const POINTS pt = MAKEPOINTS(lParam);
				MouseButtonReleasedEvent event(1, pt.x, pt.y);
				data.eventCallback(event);

				if (pt.x < 0 || pt.x >= (SHORT)m_Data.width || pt.y < 0 || pt.y >= (SHORT)m_Data.height)
				{
					ReleaseCapture();
					MouseLeftEvent event(pt.x, pt.y);
					data.eventCallback(event);
					receivingMouseInput = false;
				}
			}
			break;
		case WM_MOUSEMOVE:
			{
				const POINTS pt = MAKEPOINTS(lParam);

				input->mousePosX = pt.x;
				input->mousePosY = pt.y;

				if (pt.x >= 0 && pt.x < (SHORT)m_Data.width && pt.y >= 0 && pt.y < (SHORT)m_Data.height)
				{
					if (!receivingMouseInput)
					{
						SetCapture(m_HWnd);
						MouseEnteredEvent event(pt.x, pt.y);
						data.eventCallback(event);
						receivingMouseInput = true;
					}

					MouseMovedEvent event(pt.x, pt.y);
					data.eventCallback(event);
				}
				else if ((wParam & (MK_LBUTTON | MK_RBUTTON)) == 0 && receivingMouseInput)
				{
					ReleaseCapture();
					MouseLeftEvent event(pt.x, pt.y);
					data.eventCallback(event);
					receivingMouseInput = false;
				}

				break;
			}
		case WM_MOUSEWHEEL:
			{
				const POINTS pt = MAKEPOINTS(lParam);
				MouseScrolledEvent event(pt.x, pt.y, GET_WHEEL_DELTA_WPARAM(wParam));
				data.eventCallback(event);
				break;
			}

			/****** Raw Mouse Messages *******/
		case WM_INPUT:
			{
				UINT size;
				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(lParam),
					RID_INPUT,
					nullptr,
					&size,
					sizeof(RAWINPUTHEADER)) == -1)
				{
					break;
				}

				rawBuffer.resize(size);

				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(lParam),
					RID_INPUT,
					rawBuffer.data(),
					&size,
					sizeof(RAWINPUTHEADER)) != size)
				{
					break;
				}

				auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
				if (ri.header.dwType == RIM_TYPEMOUSE &&
					(ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
				{
					input->mouseDeltaX = ri.data.mouse.lLastX;
					input->mouseDeltaY = ri.data.mouse.lLastY;
				}
			}
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI WindowsWindow::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		if (msg == WM_NCCREATE)
		{
			//Extract pointer to WindowsWindow class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			WindowsWindow* const pWnd = static_cast<WindowsWindow*>(pCreate->lpCreateParams);

			//Set WinAPI-managed user data to store pointer to WindowsWindow class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));

			//Set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&WindowsWindow::HandleMsgThunk));

			//Forward message to window class handler
			return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
		}

		//If we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT WINAPI WindowsWindow::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		//Retreive pointer to class
		WindowsWindow* const pWnd = reinterpret_cast<WindowsWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

		//Forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}
}
