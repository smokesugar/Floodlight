#include "ApplicationInput.h"

#include "Floodlight/Utilities/Keywords.h"

namespace Floodlight {

	confined bool WindowClosed;
	confined std::optional<WindowResizeDimensions> WindowResized;

	void
	ResetApplicationEvents()
	{
		WindowClosed = false;
		WindowResized.reset();
	}

	bool
	WasWindowClosed()
	{
		return WindowClosed;
	}

	void 
	SubmitWindowClosed()
	{
		WindowClosed = true;
	}

	std::optional<WindowResizeDimensions>
	WasWindowResized()
	{
		return WindowResized;
	}

	void
	SubmitWindowResized(WindowResizeDimensions Resize)
	{
		WindowResized = Resize;
	}

}
