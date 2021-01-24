#pragma once

#include "ptpch.h"
#include "Proton/Core.h"
#include "Proton/Events/Event.h"
#include <random>
#include <DirectXMath.h>

namespace Proton
{
	class Drawable;

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
		virtual void* GetNativeWindow() const = 0;

		virtual void SetTitle(const std::string& title) = 0;
		virtual void SetProjection(DirectX::FXMMATRIX proj) noexcept = 0;

		virtual std::unique_ptr<class Box> CreateBox(std::mt19937& rng,
			std::uniform_real_distribution<float>& adist,
			std::uniform_real_distribution<float>& ddist,
			std::uniform_real_distribution<float>& odist,
			std::uniform_real_distribution<float>& rdist,
			std::uniform_real_distribution<float>& bdist) = 0;

		virtual class PointLight* CreateLight(float radius = 0.5f) = 0;

		virtual void BindLight(class PointLight* light) = 0;

		virtual void DrawLight(class PointLight* light) = 0;

		//Windows Camera, make abstract later
		virtual void SetCamera(DirectX::FXMMATRIX cam) = 0;

		virtual void InitImGui() = 0;

		virtual void Draw(Drawable* drawable) = 0;

		static Window* Create(const WindowProperties& props = WindowProperties());
	};
}