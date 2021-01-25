#include "Window.h"

#include "Floodlight/Utilities/Assert.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Input/ApplicationInput.h"

namespace Floodlight {

	/// <summary>
	/// Handles window messages and posts events.
	/// </summary>
	internal LRESULT CALLBACK
	WindowProc(HWND window, UINT Msg, WPARAM WParam, LPARAM LParam)
	{
		// Deal with the messages that we care about
		switch (Msg)
		{
		case WM_CLOSE: {
			SubmitWindowClosed();
		} break;
		}

		// Let window handle everything else
		return DefWindowProc(window, Msg, WParam, LParam);
	}

	/// <summary>
	/// Calculates actual window dimensions based on client dimensions.
	/// </summary>
	internal void
	GetWindowDimensions(uint32 ClientW, uint32 ClientH, DWORD Styles, uint32* OutW, uint32* OutH)
	{
		RECT WindowRect;
		WindowRect.left = 100;
		WindowRect.top = 100;
		WindowRect.right = ClientW + WindowRect.left;
		WindowRect.bottom = ClientH + WindowRect.top;
		AdjustWindowRect(&WindowRect, Styles, false);

		*OutW = WindowRect.right - WindowRect.left;
		*OutH = WindowRect.bottom - WindowRect.top;
	}

	/// <summary>
	/// Creates a win32 window based on a descriptor.
	/// </summary>
	HWND
	CreateWin32Window(WindowDesc Desc)
	{
		HINSTANCE HInstance = GetModuleHandle(nullptr);

		WNDCLASS WinClass = {};
		WinClass.lpfnWndProc = WindowProc;
		WinClass.hInstance = HInstance;
		WinClass.lpszClassName = "FloodlightWindowClass";
		RegisterClass(&WinClass);

		DWORD Styles = WS_OVERLAPPEDWINDOW;

		uint32 Width, Height;
		GetWindowDimensions(Desc.Width, Desc.Height, Styles, &Width, &Height);

		HWND Window = CreateWindowEx(
			0, WinClass.lpszClassName,
			Desc.Title,
			Styles,
			CW_USEDEFAULT, CW_USEDEFAULT,
			Width, Height,
			nullptr, nullptr,
			HInstance, nullptr
		);

		FL_Assert(Window, "Failed to create window.");

		ShowWindow(Window, SW_SHOW);

		return Window;
	}

	/// <summary>
	/// Flushes win32 message queue.
	/// </summary>
	void
	PollWin32Events()
	{
		MSG Message = {};
		while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

}