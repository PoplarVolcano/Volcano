#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#ifdef VOL_BUILD_DLL
#define VOL_API __declspec(dllexport)
#else
#define VOL_API __declspec(dllimport)
#endif

namespace Volcano {

	class VOL_API Log
	{
	public:
		static void Init();

		// 在 Windows 上使用 __declspec(dllexport) 导出类时，类的静态成员变量也需要导出。但 std::shared_ptr 作为标准库类型，不能直接导出。
		// 改为返回引用，内联实现
        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
            static std::shared_ptr<spdlog::logger> s_CoreLogger;
            return s_CoreLogger;
        }

        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() {
            static std::shared_ptr<spdlog::logger> s_ClientLogger;
            return s_ClientLogger;
        }
	};

}

// Core log macros(宏命令）
#define VOL_CORE_TRACE(...)	::Volcano::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VOL_CORE_INFO(...)	::Volcano::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VOL_CORE_WARN(...)	::Volcano::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VOL_CORE_ERROR(...)	::Volcano::Log::GetCoreLogger()->error(__VA_ARGS__)

// Client log macros(宏命令）
#define VOL_TRACE(...)	::Volcano::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VOL_INFO(...)	::Volcano::Log::GetClientLogger()->info(__VA_ARGS__)
#define VOL_WARN(...)	::Volcano::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VOL_ERROR(...)	::Volcano::Log::GetClientLogger()->error(__VA_ARGS__)