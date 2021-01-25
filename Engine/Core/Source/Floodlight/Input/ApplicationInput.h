#pragma once

#include <optional>

#include "Floodlight/Utilities/IntTypes.h"

namespace Floodlight {

	void ResetApplicationEvents();
	
	bool WasWindowClosed();
	void SubmitWindowClosed();

	struct WindowResizeDimensions {
		uint32 Width;
		uint32 Height;
	};

	std::optional<WindowResizeDimensions> WasWindowResized();
	void SubmitWindowResized(WindowResizeDimensions Resize);

}