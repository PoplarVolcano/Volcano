#include "volpch.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace Volcano {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init() {
		//设置log格式，颜色，时间戳，日志名，日志消息
		spdlog::set_pattern("%^[%T] %n: %v%$");
		// 用于创建一个多线程的输出到控制台的 `logger` 对象
		// 参数：logger对象的名称
		s_CoreLogger = spdlog::stdout_color_mt("Volcano");
		/*
		- `trace`：最低级别，用于记录一些系统运行状态和调试信息。
		- `debug`：用于记录调试信息、变量的值等，主要是为了方便程序员调试程序。
		- `info`：用于记录一些重要的信息，例如程序启动和退出、配置文件加载、网络连接等。
		- `warn`：用于表示某些不严重的异常或警告信息，不会影响程序运行。
		- `error`：用于表示发生了一些错误，并可能会影响程序的正常运行。
		- `critical`：最高级别，表示发生了非常严重的问题，可能会导致程序崩溃或系统崩溃。
		*/
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = spdlog::stdout_color_mt("APP");
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}