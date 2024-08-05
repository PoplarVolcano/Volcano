#pragma once

#include <memory>

/*
	__declspec(dllexport)����Windows�еĶ�̬���У�
	���������������ࡢ����ȹ�������ã�ʡ�Ը���.def�ļ���
	�����������������Ϊ����������������������ã���Ϊ��̬��Ķ���ӿں�������ȡ�
*/
#ifdef VOL_PLATFORM_WINDOWS
#if VOL_DYNAMIC_LINK
	#ifdef VOL_BUILD_DLL
		#define VOLCANO_API __declspec(dllexport)
	#else
		#define VOLCANO_API __declspec(dllimport)
	#endif
#else
	#define VOLCANO_API
#endif
#else
	#error Volcano only support Windows!
#endif

#ifdef VOL_DEBUG
	#define VOL_ENABLE_ASSERTS
#endif

#ifdef VOL_ENABLE_ASSERTS
	#define VOL_ASSERT(x, ...) { if(!(x)) { VOL_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define VOL_CORE_ASSERT(x, ...) { if(!(x)) { VOL_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define VOL_ASSERT(x, ...)
	#define VOL_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define VOL_BIND_EVENT_FN(fn) std::bind(&fn, this,std::placeholders::_1)


namespace Volcano {

	//using ����������
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;

	
}