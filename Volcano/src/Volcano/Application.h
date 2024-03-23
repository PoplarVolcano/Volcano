#pragma once

#include "Core.h"

namespace Volcano {

	class VOLCANO_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();
	};

	// To be defined in CLIENT
	Application* CreateApplication();
}