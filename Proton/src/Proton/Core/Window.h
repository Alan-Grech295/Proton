#pragma once

#include "ptpch.h"
#include "Proton\Core\Core.h"
#include "Proton/Events/Event.h"
#include <random>
#include <DirectXMath.h>
#include "Proton\Core\TimeStep.h"

namespace Proton
{
	//class Drawable;

	struct WindowProperties
	{
		std::string title;
		unsigned int width;
		unsigned int height;

		WindowProperties(const std::string& title = "Proton Game Engine",
						 unsigned int width = 1280,
						 unsigned int height = 720)
			: title(title), width(width), height(height)
		{ }
	};

	//Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate(TimeStep ts) = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		virtual void ShowCursor() const = 0;
		virtual void HideCursor() const = 0;

		virtual void EnableImGuiCursor() const = 0;
		virtual void DisableImGuiCursor() const = 0;

		virtual void ConfineCursor() const = 0;
		virtual void FreeCursor() const = 0;

		virtual void Close() = 0;

		//Window Attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void* GetNativeWindow() const = 0;

		virtual void SetTitle(const std::string& title) = 0;

		virtual void InitImGui() = 0;

		static Window* Create(const WindowProperties& props = WindowProperties());
	};
}