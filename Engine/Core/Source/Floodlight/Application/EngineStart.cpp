#include "EngineStart.h"

#include "Application.h"

namespace Floodlight {

	void EngineStart()
	{
		// Initialize core engine

		Application* Instance = CreateFloodlightInstance();

		// Tick loop

		while (true)
		{
			Instance->Tick();
		}

		// Clean up core engine

		delete Instance;
	}

}