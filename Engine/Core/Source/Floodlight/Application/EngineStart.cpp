#include "EngineStart.h"

#include "Application.h"
#include "Window.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Input/ApplicationInput.h"
#include "Floodlight/Input/MouseInput.h"
#include "Floodlight/Input/KeyboardInput.h"

namespace Floodlight {

	confined HWND
	InitializeWindow()
	{
		WindowDesc Desc = {};
		Desc.Width = 1280;
		Desc.Height = 720;
		Desc.Title = "Floodlight Engine";
		return CreateWin32Window(Desc);
	}

	confined void
	ResetAllEvents()
	{
		ResetKeyboardEvents();
		ResetMouseEvents();
		ResetApplicationEvents();
	}

	void
	EngineStart()
	{
		// Initialize core engine

		HWND Window = InitializeWindow();

		Application* Instance = CreateFloodlightInstance();

		// Tick loop

		while (!WasWindowClosed())
		{
			// Call the client's tick method
			Instance->Tick();
			// Reset and collect all events
			ResetAllEvents();
			PollWin32Events();
		}

		// Clean up core engine

		delete Instance;
	}

}