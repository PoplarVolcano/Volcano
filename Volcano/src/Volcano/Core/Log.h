#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Volcano {

	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
	};

}

// Core log macros(ºêÃüÁî£©
#define VOL_CORE_TRACE(...)	::Volcano::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define VOL_CORE_INFO(...)	::Volcano::Log::GetCoreLogger()->info(__VA_ARGS__)
#define VOL_CORE_WARN(...)	::Volcano::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define VOL_CORE_ERROR(...)	::Volcano::Log::GetCoreLogger()->error(__VA_ARGS__)

// Client log macros(ºêÃüÁî£©
#define VOL_TRACE(...)	::Volcano::Log::GetClientLogger()->trace(__VA_ARGS__)
#define VOL_INFO(...)	::Volcano::Log::GetClientLogger()->info(__VA_ARGS__)
#define VOL_WARN(...)	::Volcano::Log::GetClientLogger()->warn(__VA_ARGS__)
#define VOL_ERROR(...)	::Volcano::Log::GetClientLogger()->error(__VA_ARGS__)
