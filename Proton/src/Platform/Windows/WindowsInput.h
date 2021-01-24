#pragma once
#include "Proton/Input.h"
#include "Proton/Core.h"
#include <bitset>


namespace Proton
{
	class WindowsInput : public Input
	{
		friend class WindowsWindow;
		friend Input;
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
	private:
		static constexpr unsigned int nKeys = 256;
		static constexpr unsigned int nMouseButtons = 3;
		std::bitset<nKeys> keyStates;
		std::bitset<nMouseButtons> mbStates;
		float mousePosX;
		float mousePosY;
	private:
		static WindowsInput* s_Instance;
	};
}