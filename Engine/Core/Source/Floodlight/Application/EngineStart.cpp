#include "EngineStart.h"

#include "Application.h"
#include "Window.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Input/ApplicationInput.h"
#include "Floodlight/Input/MouseInput.h"
#include "Floodlight/Input/KeyboardInput.h"
#include "Floodlight/Renderer/D3D/D3DContext.h"

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

	confined void
	GetWindowDimensions(HWND Window, uint32* Width, uint32* Height)
	{
		LPARAM LParam = (LPARAM)GetWindowLongPtr(Window, GWLP_USERDATA);
		*Width = LOWORD(LParam);
		*Height = HIWORD(LParam);
	}

	confined double
	GetTime()
	{
		LARGE_INTEGER time, freq;
		QueryPerformanceCounter(&time);
		QueryPerformanceFrequency(&freq);
		return (double)time.QuadPart / (double)freq.QuadPart;
	}

	void
	EngineStart()
	{
		// Initialize core engine

		HWND Window = InitializeWindow();
		D3DContext::Init(Window);

		Application* Instance = CreateFloodlightInstance();

		// Tick loop

		while (!WasWindowClosed())
		{
			// Calculate delta time
			persist double LastTime = GetTime();
			double Time = GetTime();
			double DeltaTime = Time - LastTime;
			LastTime = Time;

			persist double TimeAccum = 0;
			TimeAccum += DeltaTime;

			// Call the client's tick method
			Instance->Tick();
			// Reset and collect all events
			ResetAllEvents();
			PollWin32Events();

			uint32 Width, Height;
			GetWindowDimensions(Window, &Width, &Height);
			D3DContext::Render(Width, Height, (float)TimeAccum);
		}

		// Clean up core engine

		delete Instance;

		D3DContext::Shutdown();

		#ifndef NDEBUG // If we are in debug mode, we pause when the engine shuts down so we can see any log messages.
			system("pause");
		#endif
	}

}