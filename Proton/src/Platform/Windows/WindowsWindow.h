#pragma once

#include "Proton/Window.h"
#include "Proton/Log.h"
#include <memory>
#include <random>
#include "WindowsInput.h"

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
		inline virtual void* GetNativeWindow() const override { return m_HWnd; }

		void SetTitle(const std::string& title) override;

		void InitImGui() override;

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
			bool vSync = true;
			EventCallbackFn eventCallback;
		};

		WindowData m_Data;

		HWND m_HWnd;
		HINSTANCE m_HInstance;
		bool receivingMouseInput;
		bool initializedImGui = false;
		WindowsInput* input;

		class DirectXRendererAPI* api;

		//Temp variables
		Timer timer;
	};
}