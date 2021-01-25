#include "DebugLog.h"

#include "spdlog/sinks/stdout_color_sinks.h"

#include "Keywords.h"

namespace Floodlight {

	internal std::shared_ptr<spdlog::logger> Logger;

	void
	DebugLog::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		Logger = spdlog::stdout_color_mt("Floodlight");
		Logger->set_level(spdlog::level::trace);
	}

	std::shared_ptr<spdlog::logger>&
	DebugLog::GetLogger()
	{
		return Logger;
	}

}

