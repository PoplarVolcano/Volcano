#include "volpch.h"

#include "spdlog/sinks/stdout_color_sinks.h"


namespace Volcano {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
		spdlog::set_pattern("%^[%T] %n: %v%$");//设置log格式，颜色，时间戳，日志名，日志消息
		s_CoreLogger = spdlog::stdout_color_mt("Volcano");
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}