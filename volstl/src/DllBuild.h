#pragma once

#ifdef VOLSTL_BUILD_DLL
#define VOLSTL_API __declspec(dllexport)
#else
#define VOLSTL_API __declspec(dllimport)
#endif

// 导出一个初始化函数（强制生成 .lib）
VOLSTL_API void VolstlInit();