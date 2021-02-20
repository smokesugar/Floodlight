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
		D3DContext::BeginFrame();

		Application* Instance = CreateFloodlightInstance();

		// Tick loop

		while (!WasWindowClosed())
		{
			Time::NewFrame();
			Instance->Tick();

			D3DContext::EndFrame();

			// Reset and collect all events
			ResetAllEvents();
			PollWin32Events();

			D3DContext::BeginFrame();
		}

		// Clean up core engine

		delete Instance;

		D3DContext::Shutdown();

		#ifndef NDEBUG // If we are in debug mode, we pause when the engine shuts down so we can see any log messages.
			system("pause");
		#endif
	}

}