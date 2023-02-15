#pragma once
#include "Proton\Core\Window.h"
#include <memory>
#include <random>
#include "WindowsInput.h"

namespace Proton
{	
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProperties& props, HINSTANCE hInstance);
		virtual ~WindowsWindow();

		void OnUpdate(TimeStep ts) override;

		inline unsigned int GetWidth() const override { return m_Data.width; }
		inline unsigned int GetHeight() const override { return m_Data.height; }

		virtual void ShowCursor() const override;
		virtual void HideCursor() const override;

		virtual void EnableImGuiCursor() const override;
		virtual void DisableImGuiCursor() const override;

		virtual void ConfineCursor() const override;
		virtual void FreeCursor() const override;

		//Window Attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.eventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;
		inline virtual void* GetNativeWindow() const override { return m_HWnd; }

		virtual void Shutdown();
		virtual void Close() override;

		void SetTitle(const std::string& title) override;

		void InitImGui() override;

		static LRESULT WINAPI HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT WINAPI HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT WINAPI HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

		//Windows Only calls
		void ClearKeys();
	private:
		virtual void Init(const WindowProperties& props, HINSTANCE hInstance);
	private:
		struct WindowData
		{
			std::string title = "";
			unsigned int width = 0;
			unsigned int height = 0;
			bool vSync = true;
			EventCallbackFn eventCallback;
		};

		WindowData m_Data;

		HWND m_HWnd;
		HINSTANCE m_HInstance;
		bool receivingMouseInput;
		bool initializedImGui = false;
		WindowsInput* input;

		std::vector<BYTE> rawBuffer;
	};
}