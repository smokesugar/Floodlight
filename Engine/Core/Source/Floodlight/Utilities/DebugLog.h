#pragma once

#include "spdlog/spdlog.h"

namespace Floodlight {
	
	namespace DebugLog {
		void Init();
		std::shared_ptr<spdlog::logger>& GetLogger();
	}

}

#ifdef NDEBUG
#define FL_Trace(...)   
#define FL_Info(...)    
#define FL_Warn(...)    
#define FL_Error(...)   
#define FL_Critical(...)
#else
#define FL_Trace(...)     ::Floodlight::DebugLog::GetLogger()->trace(__VA_ARGS__);
#define FL_Info(...)      ::Floodlight::DebugLog::GetLogger()->info(__VA_ARGS__);
#define FL_Warn(...)      ::Floodlight::DebugLog::GetLogger()->warn(__VA_ARGS__);
#define FL_Error(...)     ::Floodlight::DebugLog::GetLogger()->error(__VA_ARGS__);
#define FL_Fatal(...)  ::Floodlight::DebugLog::GetLogger()->critical(__VA_ARGS__);
#endif