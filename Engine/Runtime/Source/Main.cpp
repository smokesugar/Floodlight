#include "Runtime.h"

#include "Floodlight/Application/EntryPoint.h"

namespace Floodlight {

	Application*
	CreateFloodlightInstance()
	{
		return new Runtime();
	}

}