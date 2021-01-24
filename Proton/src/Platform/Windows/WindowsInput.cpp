#include "ptpch.h"
#include "WindowsInput.h"
#include "WindowsWindow.h"
#include <utility>

namespace Proton
{
	WindowsInput* WindowsInput::s_Instance = new WindowsInput();
	Input* Input::s_Instance = WindowsInput::s_Instance;

	bool WindowsInput::IsKeyPressedImpl(int keycode)
	{
		return keyStates[keycode];
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
}