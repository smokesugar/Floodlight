#include "EngineStart.h"

#include "Application.h"
#include "Window.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Input/ApplicationInput.h"

namespace Floodlight {

	internal HWND
	InitializeWindow()
	{
		WindowDesc Desc = {};
		Desc.Width = 1280;
		Desc.Height = 720;
		Desc.Title = "Floodlight Engine";
		return CreateWin32Window(Desc);
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
			Instance->Tick();
			PollWin32Events();
		}

		// Clean up core engine

		delete Instance;
	}

}