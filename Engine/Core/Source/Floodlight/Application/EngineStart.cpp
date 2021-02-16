#include "EngineStart.h"

#include "Application.h"
#include "Window.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Input/ApplicationInput.h"
#include "Floodlight/Input/MouseInput.h"
#include "Floodlight/Input/KeyboardInput.h"
#include "Floodlight/Renderer/D3D/D3DContext.h"
#include "Time.h"

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

		GetMainWindow() = InitializeWindow();
		D3DContext::Init(GetMainWindow());

		Application* Instance = CreateFloodlightInstance();

		// Tick loop

		while (!WasWindowClosed())
		{
			Time::NewFrame();

			D3DContext::BeginFrame();
			Instance->Tick();
			D3DContext::EndFrame();

			// Reset and collect all events
			ResetAllEvents();
			PollWin32Events();
		}

		// Clean up core engine

		delete Instance;

		D3DContext::Shutdown();

		#ifndef NDEBUG // If we are in debug mode, we pause when the engine shuts down so we can see any log messages.
			system("pause");
		#endif
	}

}