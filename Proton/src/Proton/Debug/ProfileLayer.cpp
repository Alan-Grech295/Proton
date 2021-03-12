#include "ptpch.h"
#include "ProfileLayer.h"
#include "Proton\Core\Input.h"
#include "Proton\Core\KeyCodes.h"

namespace Proton
{
	ProfileLayer::ProfileLayer()
		:
		Layer("ProfileLayer")
	{
		Instrumentor::StopProfiling();
		profiling = Instrumentor::Profiling();
	}

	ProfileLayer::~ProfileLayer()
	{
	}

	void ProfileLayer::OnUpdate(TimeStep ts)
	{
		if (Input::IsKeyReleased(PT_KEY_P))
		{
			profiling = !profiling;
			Instrumentor::SetProfiling(profiling);
		}

		if (Input::IsKeyReleased(PT_KEY_O) && !profiling)
		{
			profileMode++;
			if (profileMode > 1)
				profileMode = 0;
		}

		if (profiling && profileMode == 1)
		{
			currentFrame++;
			if (currentFrame > profileFrames)
			{
				profiling = false;
				Instrumentor::SetProfiling(false);
				currentFrame = 0;
			}
		}
	}

	void ProfileLayer::OnImGuiRender()
	{
		if (ImGui::Begin("Profiler"))
		{
			char buffer[64];
			strncpy(buffer, "Profiler Status: ", sizeof(buffer));
			strncat(buffer, (Instrumentor::Profiling() ? "Running" : "Stopped"), sizeof(buffer));

			ImGui::Text(buffer);

			strncpy(buffer, "Profiler Mode: ", sizeof(buffer));
			strncat(buffer, GetProfileMode(), sizeof(buffer));

			ImGui::Text(buffer);

			if(profileMode == 1)
				ImGui::SliderInt("Profile Frames", &profileFrames, 1, 512);
		}

		ImGui::End();
	}
}
