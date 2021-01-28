#pragma once

#include "Proton/Window.h"
#include "Proton/Log.h"
#include <memory>
#include "WindowsGraphics.h"
#include <random>
#include "WindowsInput.h"

//Temp includes
#include "Proton/Timer.h"

namespace Proton
{
	class WindowsGraphics;
	//class Drawable;

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
		inline virtual void* GetNativeWindow() const { return m_HWnd; }

		void SetTitle(const std::string& title) override;
		void SetProjection(DirectX::FXMMATRIX proj) noexcept override;

		std::unique_ptr<class Box> CreateBox(std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist,
			std::uniform_real_distribution<float>& bdist,
			DirectX::XMFLOAT3 material) override;

		virtual std::unique_ptr<class AssimpTest> CreateTestMesh(
			std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist,
			DirectX::XMFLOAT3 material
		) override;

		void InitImGui() override;

		void SetCamera(DirectX::FXMMATRIX cam) override;

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
			bool vSync = true;
			EventCallbackFn eventCallback;
		};

		WindowData m_Data;

		HWND m_HWnd;
		HINSTANCE m_HInstance;
		bool receivingMouseInput;
		bool initializedImGui = false;
		std::unique_ptr<WindowsGraphics> pGfx;
		WindowsInput* input;

		//Temp variables
		Timer timer;
	};
}