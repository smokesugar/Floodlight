#pragma once

#include <optional>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	enum class MouseButton {
		LBUTTON = 0x01,
		MBUTTON = 0x04,
		RBUTTON = 0x02
	};

	void ResetMouseEvents();

	struct MouseMovePositions {
		uint32 XPos;
		uint32 YPos;
	};

	std::optional<MouseMovePositions> WasMouseMoved();
	void SubmitMouseMoved(MouseMovePositions Move);

	bool IsMouseButtonPressed(MouseButton Button);

	bool WasAnyMouseButtonPressed();
	bool WasAnyMouseButtonReleased();

	bool WasMouseButtonPressed(MouseButton Button);
	void SubmitMouseButtonPressed(MouseButton Button);

	bool WasMouseButtonReleased(MouseButton Button);
	void SubmitMouseButtonReleased(MouseButton Button);

}