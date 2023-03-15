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

		void BlockEvents(bool block) { m_BlockEvents = block; }

		void SetDarkThemeColors();

		void OnEvent(Event& e) override;
	private:
		bool m_BlockEvents = false;
	};
}