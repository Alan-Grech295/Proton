#pragma once

#include "Core.h"

namespace Proton
{
	class PROTON_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	//To be defined in CLIENT
	Application* CreateApplication();
}

