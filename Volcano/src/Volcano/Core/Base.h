#pragma once

#include <memory>

namespace Volcano {

	void InitializeCore();
	void ShutdownCore();
}

#ifndef VOL_PLATFORM_WINDOWS
	#define VOL_PLATFORM_WINDOWS
#endif

// __VA_ARGS__ expansion to get past MSVC "bug"
//‌ VA_ARGS是一个可变参数的宏，是C99规范中新增的。 
// 它允许宏接收不同数量的参数，并且在宏定义中表示所有传递给宏的参数。
// 这个宏通常与宏函数一起使用，以便在宏中处理不定数量的参数。‌
#define VOL_EXPAND_VARGS(x) x

//二进制，得到2^(x-1)
#define BIT(x) (1 << x)

// bind函数是C++11标准中的一个函数模板，用于将函数和一组参数绑定在一起，
// 生成一个新的可调用对象。可以用于实现函数适配器、参数绑定等功能。
// bind函数的返回值是一个可调用对象，可以通过调用该对象来执行绑定的函数。
// std::placeholders 是 C++ 标准库中的一个命名空间，它包含了一组特殊的占位符对象，用于与 std::bind 函数一起使用。
// 这些占位符对象允许你在绑定函数时表示某些参数是未指定的，并在稍后提供这些参数的值。
// std::placeholders 中有以下占位符：
// std::placeholders::_1：表示第一个未指定的参数。
// std::placeholders::_2：表示第二个未指定的参数。
#define VOL_BIND_EVENT_FN(fn) std::bind(&fn, this,std::placeholders::_1)

#include "Log.h"
#include "Assert.h"

// Pointer wrappers 指针包装
namespace Volcano {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	using byte = uint8_t;

}