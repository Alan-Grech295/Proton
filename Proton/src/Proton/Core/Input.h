#pragma once
#include "Core.h"
#include "KeyCodes.h"

namespace Proton
{
	class Input
	{
	public:
		inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsKeyReleased(KeyCode keycode) { return s_Instance->IsKeyReleasedImpl(keycode); }

		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static float GetMouseDeltaX() { return s_Instance->GetMouseXDeltaImpl(); }
		inline static float GetMouseDeltaY() { return s_Instance->GetMouseYDeltaImpl(); }
	protected:
		virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;
		virtual bool IsKeyReleasedImpl(KeyCode keycode) = 0;

		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
		virtual float GetMouseXDeltaImpl() = 0;
		virtual float GetMouseYDeltaImpl() = 0;
	private:
		static Input* s_Instance;
	};
}