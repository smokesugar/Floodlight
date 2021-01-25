#include "ApplicationInput.h"

#include "Floodlight/Utilities/Keywords.h"

namespace Floodlight {

	internal bool WindowClosed = false;

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

}
