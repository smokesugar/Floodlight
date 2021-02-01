#include "DebugLog.h"

#include "spdlog/sinks/stdout_color_sinks.h"

#include "Keywords.h"

namespace Floodlight {

	void
	DebugLog::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		GetLogger() = spdlog::stdout_color_mt("Floodlight");
		GetLogger()->set_level(spdlog::level::trace);
	}

	std::shared_ptr<spdlog::logger>&
	DebugLog::GetLogger()
	{
		persist std::shared_ptr<spdlog::logger> Logger;
		return Logger;
	}

}

