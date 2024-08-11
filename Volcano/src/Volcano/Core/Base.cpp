#include "volpch.h"
#include "Base.h"


#define VOLCANO_BUILD_ID "v0.1a"

namespace Volcano {

	void InitializeCore()
	{
		//初始化日志
		Volcano::Log::Init();

		VOL_CORE_WARN("Initialized Log!");
		int a = 5;
		VOL_INFO("Hello! Var={0}", a);

		VOL_CORE_TRACE("Volcano Engine {}", VOLCANO_BUILD_ID);
		VOL_CORE_TRACE("Initializing...");
	}

	void ShutdownCore()
	{
		VOL_CORE_TRACE("Shutting down...");
	}
}