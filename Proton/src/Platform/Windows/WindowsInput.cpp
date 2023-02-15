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
		return mbStates[button];
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
}