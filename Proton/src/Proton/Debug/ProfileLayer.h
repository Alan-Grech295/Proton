#pragma once
#include "Proton\Core\Layer.h"
#include "Instrumentation.h"
#include <imgui.h>

namespace Proton
{
	class ProfileLayer : public Layer
	{
	public:
		ProfileLayer();
		~ProfileLayer();

		const char* GetProfileMode()
		{
			switch (profileMode)
			{
			case 0:
				return "Toggle";
			case 1:
				return "Set Frames";
			}

			return "Unknown";
		}

		virtual void OnUpdate(TimeStep ts) override;
		virtual void OnImGuiRender() override;

		static int mouseUpdates;

	private:
		bool profiling = false;
		int profileMode = 0;
		int profileFrames = 10;
		int currentFrame = 0;
	};
}