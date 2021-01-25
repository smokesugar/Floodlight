#include "MouseInput.h"

#include <unordered_set>

#include "Floodlight/Utilities/WinIncLean.h"
#include "Floodlight/Utilities/Keywords.h"
#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	confined std::optional<MouseMovePositions> MouseMoved;
	confined std::unordered_set<MouseButton> MouseButtonPressedSet;
	confined std::unordered_set<MouseButton> MouseButtonReleasedSet;

	void ResetMouseEvents()
	{
		MouseMoved.reset();
		MouseButtonPressedSet.clear();
		MouseButtonReleasedSet.clear();
	}

	std::optional<MouseMovePositions>
	WasMouseMoved()
	{
		return MouseMoved;
	}

	void
	SubmitMouseMoved(MouseMovePositions Move)
	{
		MouseMoved = Move;
	}

	bool
	IsMouseButtonPressed(MouseButton Button)
	{
		return GetKeyState((uint16)Button) & 0x8000;
	}

	bool
	WasAnyMouseButtonPressed()
	{
		return !MouseButtonPressedSet.empty();
	}

	bool
	WasAnyMouseButtonReleased()
	{
		return !MouseButtonReleasedSet.empty();
	}

    bool
	WasMouseButtonPressed(MouseButton Button)
    {
		return MouseButtonPressedSet.find(Button) != MouseButtonPressedSet.end();
    }

    void
	SubmitMouseButtonPressed(MouseButton Button)
    {
		MouseButtonPressedSet.insert(Button);
    }

	bool
	WasMouseButtonReleased(MouseButton Button)
	{
		return MouseButtonReleasedSet.find(Button) != MouseButtonReleasedSet.end();
	}

	void
	SubmitMouseButtonReleased(MouseButton Button)
	{
		MouseButtonReleasedSet.insert(Button);
	}

}