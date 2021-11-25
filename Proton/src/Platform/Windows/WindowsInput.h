#pragma once
#include "Proton\Core\Input.h"
#include "Proton\Core\Core.h"
#include <bitset>


namespace Proton
{
	class WindowsInput : public Input
	{
		friend class WindowsWindow;
		friend Input;
	protected:
		virtual bool IsKeyPressedImpl(int keycode) override;
		virtual bool IsKeyReleasedImpl(int keycode) override;
		virtual bool IsMouseButtonPressedImpl(int button) override;
		virtual float GetMouseXImpl() override;
		virtual float GetMouseYImpl() override;
		virtual float GetMouseXDeltaImpl() override;
		virtual float GetMouseYDeltaImpl() override;
	private:
		static constexpr unsigned int nKeys = 256;
		static constexpr unsigned int nMouseButtons = 3;
		std::bitset<nKeys> pressedKeyStates;
		std::bitset<nKeys> releasedKeyStates;
		std::bitset<nMouseButtons> mbStates;
		float mousePosX = 0;
		float mousePosY = 0;
		float mouseDeltaX = 0;
		float mouseDeltaY = 0;

		float accMouseDeltaX = 0;
		float accMouseDeltaY = 0;
	private:
		static WindowsInput* s_Instance;
	};
}