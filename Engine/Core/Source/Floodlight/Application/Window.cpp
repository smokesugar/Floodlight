#include "Window.h"

#include "Floodlight/Utilities/Assert.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Input/ApplicationInput.h"
#include "Floodlight/Input/MouseInput.h"
#include "Floodlight/Input/KeyboardInput.h"
#include "Floodlight/Renderer/D3D/D3DContext.h"

namespace Floodlight {

	/*
		Handles window messages and posts events.
	*/
	confined LRESULT CALLBACK
	WindowProc(HWND Window, UINT Msg, WPARAM WParam, LPARAM LParam)
	{
		// Deal with the messages that we care about
		switch (Msg)
		{
		// Window Closed
		case WM_CLOSE: {
			SubmitWindowClosed();
		} break;

		// Window Resized
		case WM_SIZE: {
			WindowResizeDimensions Resize;
			Resize.Width = LOWORD(LParam);
			Resize.Height = HIWORD(LParam);
			SubmitWindowResized(Resize);
			SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR)LParam);
			if (D3DContext::IsInitialized())
				D3DContext::ResizeSwapChain(Resize.Width, Resize.Height);
		} break;

		// Mouse moved
		case WM_MOUSEMOVE: {
			POINTS Pos = MAKEPOINTS(LParam);
			SubmitMouseMoved({ (uint32)Pos.x, (uint32)Pos.y });
		} break;

		// Mouse button pressed
		case WM_LBUTTONDOWN: {
			SubmitMouseButtonPressed(MouseButton::LBUTTON);
		} break;
		case WM_MBUTTONDOWN: {
			SubmitMouseButtonPressed(MouseButton::MBUTTON);
		} break;
		case WM_RBUTTONDOWN: {
			SubmitMouseButtonPressed(MouseButton::RBUTTON);
		} break;

		// Mouse button released
		case WM_LBUTTONUP: {
			SubmitMouseButtonReleased(MouseButton::LBUTTON);
		} break;
		case WM_MBUTTONUP: {
			SubmitMouseButtonReleased(MouseButton::MBUTTON);
		} break;
		case WM_RBUTTONUP: {
			SubmitMouseButtonReleased(MouseButton::RBUTTON);
		} break;

		// Key Pressed
		case WM_SYSKEYDOWN:
		case WM_KEYDOWN: {
			Key K = (Key)WParam;
			SubmitKeyPressed(K);
		} break;
		
		// Key Released
		case WM_SYSKEYUP:
		case WM_KEYUP: {
			Key K = (Key)WParam;
			SubmitKeyReleased(K);
		} break;
		}

		// Let window handle everything else
		return DefWindowProc(Window, Msg, WParam, LParam);
	}

	/*
		Calculates actual window dimensions based on client dimensions.
	*/
	confined void
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

	/*
		Creates a win32 window based on a descriptor.
	*/
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

	/*
		Flushes win32 message queue.
	*/
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