#include "ptpch.h"
#include "WindowsInput.h"
#include "WindowsWindow.h"
#include <utility>

namespace Proton
{
	WindowsInput* WindowsInput::s_Instance = new WindowsInput();
	Input* Input::s_Instance = WindowsInput::s_Instance;

	bool WindowsInput::IsKeyPressedImpl(KeyCode keycode)
	{
		return pressedKeyStates[keycode];
	}

	bool WindowsInput::IsKeyReleasedImpl(KeyCode keycode)
	{
		return releasedKeyStates[keycode];
	}

	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		return mbPressedStates[button];
	}

	bool WindowsInput::IsMouseButtonReleasedImpl(int button)
	{
		return mbReleasedStates[button];
	}

	float WindowsInput::GetMouseXImpl()
	{
		return mousePosX;
	}

	float WindowsInput::GetMouseYImpl()
	{
		return mousePosY;
	}

	float WindowsInput::GetMouseXDeltaImpl()
	{
		return mouseDeltaX;
	}

	float WindowsInput::GetMouseYDeltaImpl()
	{
		return mouseDeltaY;
	}
	void WindowsInput::ResetState()
	{
		mouseDeltaX = 0;
		mouseDeltaY = 0;

		accMouseDeltaX = 0;
		accMouseDeltaY = 0;

		releasedKeyStates.reset();
		mbReleasedStates.reset();
	}

	void WindowsInput::KeyDown(int keycode)
	{
		pressedKeyStates[keycode] = true;
	}

	void WindowsInput::KeyUp(int keycode)
	{
		pressedKeyStates[keycode] = false;
		releasedKeyStates[keycode] = true;
	}

	void WindowsInput::MouseButtonDown(int mb)
	{
		mbPressedStates[mb] = true;
	}

	void WindowsInput::MouseButtonUp(int mb)
	{
		mbPressedStates[mb] = false;
		mbReleasedStates[mb] = true;
	}
}