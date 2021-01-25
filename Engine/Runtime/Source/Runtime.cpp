#include "Runtime.h"

#include "Floodlight/Utilities/DebugLog.h"

namespace Floodlight {

	Runtime::Runtime()
	{
		FL_Trace("Hello World!");
		FL_Info("Hello World!");
		FL_Warn("Hello World!");
		FL_Error("Hello World!");
		FL_Critical("Hello World!");
	}

	Runtime::~Runtime()
	{
		FL_Trace("Goodbye World!");
	}

	void Runtime::Tick()
	{
		FL_Trace("Tick!");
	}

}