#pragma once

#ifdef VOL_PLATFORM_WINDOWS
	#ifdef VOL_BUILD_DLL
		#define VOLCANO_API __declspec(dllexport)
	#else
		#define VOLCANO_API __declspec(dllimport)
	#endif
#else
	#error Volcano only support Windows!
#endif