#pragma once
#include "Proton/Core.h"

namespace Proton
{
	class Input
	{
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsKeyReleased(int keycode) { return s_Instance->IsKeyReleasedImpl(keycode); }

		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static float GetMouseDeltaX() { return s_Instance->GetMouseXDeltaImpl(); }
		inline static float GetMouseDeltaY() { return s_Instance->GetMouseYDeltaImpl(); }
	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual bool IsKeyReleasedImpl(int keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
		virtual float GetMouseXDeltaImpl() = 0;
		virtual float GetMouseYDeltaImpl() = 0;
	private:
		static Input* s_Instance;
	};
}