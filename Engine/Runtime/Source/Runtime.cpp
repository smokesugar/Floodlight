#include "Runtime.h"

#include <string>

#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Utilities/DebugLog.h"
#include "Floodlight/Input/ApplicationInput.h"
#include "Floodlight/Input/MouseInput.h"
#include "Floodlight/Input/KeyboardInput.h"

namespace Floodlight {

	Runtime::Runtime()
	{
		
	}

	Runtime::~Runtime()
	{
		
	}

	void Runtime::Tick()
	{
		if (auto Resize = WasWindowResized())
			FL_Trace("Window resized: {0} {1}", Resize->Width, Resize->Height);

		if (auto Move = WasMouseMoved())
			FL_Trace("Mouse moved: {0} {1}", Move->XPos, Move->YPos);

		if (WasMouseButtonPressed(MouseButton::LBUTTON))
			FL_Trace("LButton down.");
		if (WasMouseButtonPressed(MouseButton::MBUTTON))
			FL_Trace("MButton down.");
		if (WasMouseButtonPressed(MouseButton::RBUTTON))
			FL_Trace("RButton down.");

		if (WasMouseButtonReleased(MouseButton::LBUTTON))
			FL_Trace("LButton up.");
		if (WasMouseButtonReleased(MouseButton::MBUTTON))
			FL_Trace("MButton up.");
		if (WasMouseButtonReleased(MouseButton::RBUTTON))
			FL_Trace("RButton up.");

		if (WasKeyPressed(Key::SPACE))
			FL_Trace("SPACE down.");
		if (WasKeyReleased(Key::SPACE))
			FL_Trace("SPACE up.");

		if (WasAnyKeyPressed())
			FL_Trace("Key down.");
		if (WasAnyKeyReleased())
			FL_Trace("Key up.");

		if (IsKeyPressed(Key::LSHIFT))
			FL_Info("LSHIFT is down.");

		if (IsMouseButtonPressed(MouseButton::MBUTTON))
			FL_Info("MButton is down.");
	}

}