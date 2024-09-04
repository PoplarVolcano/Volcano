#include "volpch.h"
#include "Base.h"


#define VOLCANO_BUILD_ID "v1.0"

namespace Volcano {

	void InitializeCore()
	{
		//初始化日志
		Volcano::Log::Init();

		VOL_CORE_TRACE("Volcano Engine {}", VOLCANO_BUILD_ID);
		VOL_CORE_TRACE("Volcano Engine Initializing...");
	}

	void ShutdownCore()
	{
		VOL_CORE_TRACE("Shutting down...");
	}
}