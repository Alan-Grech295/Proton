#pragma once
#include "Proton\Core\Layer.h"

namespace Proton
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

		void SetDarkThemeColors();
	};
}