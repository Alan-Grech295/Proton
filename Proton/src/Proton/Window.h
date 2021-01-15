#pragma once

#include "ptpch.h"
#include "Proton/Core.h"
#include "Proton/Events/Event.h"

namespace Proton
{
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
	class PROTON_API Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		//Window Attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;
		virtual void SetTitle(const std::string& title) = 0;
		virtual void DrawTestCube(float angle, float x, float z) = 0;

		static Window* Create(const WindowProperties& props = WindowProperties());
	};
}